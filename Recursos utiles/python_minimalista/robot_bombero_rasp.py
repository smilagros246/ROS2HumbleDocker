"""
@file robot_bombero_rasp.py
@autor Sofía Milagros Castaño Vanegas - Equipo robot bombero
@date 2025-04-08
@version 1.0
@brief Control teleoperado de robot bombero (opción intercambio entre locomoción y aspersión para uso mediante terminal con ssh).
"""
import sys
import termios
import tty
import time
import threading
import serial
from select import select

class RobotController:
    def __init__(self):
        self.speed = 200
        self.key_pressed = None
        self.last_key_time = time.time()
        self.movement_active = False
        self.mode = "LOCOMOCION"
        self.servo_angles = [90, 90, 90, 90]
        self.pump_level = "APAGADO"
        self.ir_values = [1000, 1000, 1000]
        self.imu_values = [0.0, 0.0, 0.0]
        self.IR_THRESHOLD = 1200
        self.running = True

        try:
            self.ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
            print(" Serial port abierto en /dev/ttyUSB0")
        except serial.SerialException as e:
            print(f" Error abriendo el puerto serial: {e}")
            exit(1)

        self.print_controls()

    def print_controls(self):
        print("""
╔═════════════════════════════════════╗
║   CONTROLES INTERACTIVOS DEL ROBOT ║
╚═════════════════════════════════════╝

▶ Modo actual: LOCOMOCIÓN o BRAZO (TAB para cambiar)

[LOCOMOCIÓN]
    W  →  Avanzar
    S  →  Retroceder
    A  →  Giro Izquierda
    D  →  Giro Derecha
    Q  →  Rotar antihorario
    E  →  Rotar horario
    Z/C →  Diagonales
    X  →  Detenerse
    +/- → Ajustar velocidad

[BRAZO]
    i/k → Servo 1 ↑↓
    j/l → Servo 2 ↑↓
    u/o → Servo 3 ↑↓
    n/m → Servo 4 ↑↓
    1/2/3 → Bomba (Bajo/Medio/Alto)

ESC → Salir
""")

    def get_movement(self, key):
        s = self.speed
        return {
            'w': [s, s, s, s],
            's': [-s, -s, -s, -s],
            'a': [-s, s, s, -s],
            'd': [s, -s, -s, s],
            'q': [-s, -s, s, s],
            'e': [s, s, -s, -s],
            'z': [0, s, s, 0],
            'c': [s, 0, 0, s],
            'x': [0, 0, 0, 0]
        }.get(key, None)

    def send_command(self, wheels, servos, pump):
        cmd = f"CMD:WHEELS:{','.join(map(str, wheels))};SERVOS:{','.join(map(str, servos))};PUMP:{pump}\n"
        self.ser.write(cmd.encode())
        time.sleep(0.01)
        print(f"Enviado: {cmd.strip()}")

    def toggle_mode(self):
        self.mode = "BRAZO" if self.mode == "LOCOMOCION" else "LOCOMOCION"
        self.pump_level = "APAGADO"
        print(f"Modo cambiado a: {self.mode}")

    def read_serial(self):
        while self.running:
            try:
                line = self.ser.readline().decode().strip()
                if line.startswith("IR:"):
                    self.ir_values = list(map(int, line[3:].split(',')))
                elif line.startswith("IMU:"):
                    self.imu_values = list(map(float, line[4:].split(',')))
                    if len(self.imu_values) != 6:
                        continue
                print(f"IR: {self.ir_values} | IMU: {self.imu_values}", end='\r')
            except Exception as e:
                print(f"Error serial: {e}")
                continue

    def get_char_nonblocking(self):
        # Lee un carácter sin bloquear el flujo
        dr, _, _ = select([sys.stdin], [], [], 0)
        if dr:
            return sys.stdin.read(1)
        return None

    def run(self):
        threading.Thread(target=self.read_serial, daemon=True).start()
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        tty.setcbreak(fd)

        print("Presiona teclas. ESC para salir.")

        last_key = None  # Guarda la última tecla enviada

        try:
            while self.running:
                key = self.get_char_nonblocking()
                now = time.time()

                if key:
                    if key == '\x1b':  # ESC
                        print("Saliendo...")
                        self.running = False
                        break
                    elif key == '\t':  # TAB
                        self.toggle_mode()
                    elif key == '+':
                        self.speed = min(255, self.speed + 10)
                        print(f"Velocidad: {self.speed}")
                    elif key == '-':
                        self.speed = max(0, self.speed - 10)
                        print(f"Velocidad: {self.speed}")
                    else:
                        # Detectar cambio de tecla y frenar antes
                        if key != last_key and last_key is not None:
                            # Enviar parada inmediata para tecla anterior
                            self.send_command([0, 0, 0, 0], self.servo_angles, "APAGADO")
                            time.sleep(0.02)  # pequeña pausa para evitar solapamientos

                        self.key_pressed = key
                        self.last_key_time = now

                        if self.mode == "LOCOMOCION":
                            movement = self.get_movement(key)
                            if movement:
                                if max(self.ir_values) > self.IR_THRESHOLD:
                                    print("Obstáculo detectado")
                                    movement = [0, 0, 0, 0]
                                self.send_command(movement, self.servo_angles, "APAGADO")
                                self.movement_active = True

                        elif self.mode == "BRAZO":
                            k = key
                            if k == 'i': self.servo_angles[0] = min(180, self.servo_angles[0] + 5)
                            if k == 'k': self.servo_angles[0] = max(0, self.servo_angles[0] - 5)
                            if k == 'j': self.servo_angles[1] = min(180, self.servo_angles[1] + 5)
                            if k == 'l': self.servo_angles[1] = max(0, self.servo_angles[1] - 5)
                            if k == 'u': self.servo_angles[2] = min(180, self.servo_angles[2] + 5)
                            if k == 'o': self.servo_angles[2] = max(0, self.servo_angles[2] - 5)
                            if k == 'n': self.servo_angles[3] = min(180, self.servo_angles[3] + 5)
                            if k == 'm': self.servo_angles[3] = max(0, self.servo_angles[3] - 5)
                            if k == '1': self.pump_level = "PWM_BAJO"
                            if k == '2': self.pump_level = "PWM_MEDIO"
                            if k == '3': self.pump_level = "PWM_ALTO"
                            self.send_command([0, 0, 0, 0], self.servo_angles, self.pump_level)

                        last_key = key

                # Manejo de liberación de tecla por timeout
                if self.key_pressed and (now - self.last_key_time) > 0.5:
                    self.send_command([0, 0, 0, 0], self.servo_angles, "APAGADO")
                    self.key_pressed = None
                    self.movement_active = False
                    self.pump_level = "APAGADO"
                    last_key = None  # Resetear last_key al soltar tecla

                time.sleep(0.05)

        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
            self.ser.close()
            print("Serial cerrado.")


# MAIN
if __name__ == "__main__":
    controller = RobotController()
    controller.run()
