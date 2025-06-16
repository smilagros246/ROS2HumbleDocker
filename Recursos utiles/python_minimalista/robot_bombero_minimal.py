"""
@file robot_bombero_minimal.py
@autor Sofía Milagros Castaño Vanegas - Equipo robot bombero
@date 2025-04-08
@version 1.0
@brief Control teleoperado de robot bombero (opción intercambio entre locomoción y aspersión).
"""
import serial
import time
from pynput import keyboard
import threading

class RobotController:
    def __init__(self):
        # Configuración inicial
        self.speed = 200
        self.key_pressed = None
        self.movement_active = False
        self.mode = "LOCOMOCION"
        self.servo_angles = [90, 90, 90, 90]
        self.pump_level = "APAGADO"
        self.ir_values = [1000, 1000, 1000]
        self.imu_values = [0.0, 0.0, 0.0]
        self.IR_THRESHOLD = 200

        # Serial
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

    def on_press(self, key):
        try:
            k = key.char

            if self.mode == "LOCOMOCION":
                if k == '+':
                    self.speed = min(255, self.speed + 10)
                    print(f"Velocidad aumentada: {self.speed}")
                elif k == '-':
                    self.speed = max(0, self.speed - 10)
                    print(f"Velocidad reducida: {self.speed}")
                else:
                    movement = self.get_movement(k)
                    if movement:
                        print(f"[DEBUG] IR values: {self.ir_values} | Max: {max(self.ir_values)}")
                        self.key_pressed = k
                        if max(self.ir_values) > self.IR_THRESHOLD:
                            print(" Obstáculo detectado. Movimiento cancelado.")
                            movement = [0, 0, 0, 0]
                        self.send_command(movement, self.servo_angles, "APAGADO")
                        self.movement_active = True

            elif self.mode == "BRAZO":
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

        except AttributeError:
            if key == keyboard.Key.tab:
                self.toggle_mode()
            elif key == keyboard.Key.esc:
                print("\n Terminando...")
                self.ser.close()
                return False

    def on_release(self, key):
        if self.mode == "LOCOMOCION" and self.movement_active:
            self.send_command([0, 0, 0, 0], self.servo_angles, "APAGADO")
            self.movement_active = False
            self.pump_level = "APAGADO"
        elif self.mode == "LOCOMOCION" and not self.movement_active:
            if self.pump_level != "APAGADO":
                print(" Bomba apagada por inactividad de movimiento.")
                self.send_command([0, 0, 0, 0], self.servo_angles, "APAGADO")
                self.pump_level = "APAGADO"
        elif self.mode == "BRAZO":
            if self.pump_level != "APAGADO":
                print(" Bomba apagada.")
                self.send_command([0, 0, 0, 0], self.servo_angles, "APAGADO")
                self.pump_level = "APAGADO"

    def toggle_mode(self):
        self.mode = "BRAZO" if self.mode == "LOCOMOCION" else "LOCOMOCION"
        self.pump_level = "APAGADO"
        print(f" Modo cambiado a: {self.mode}")

    def read_serial(self):
        while True:
            try:
                line = self.ser.readline().decode().strip()  # Lee la línea completa desde el puerto serie
                if line.startswith("IR:"):
                    self.ir_values = list(map(int, line[3:].split(',')))  # Extrae y convierte los valores de los sensores IR a enteros
                elif line.startswith("IMU:"):
                    imu_data = line[4:].split(',')
                    if len(imu_data) != 6 or not all(val.strip().replace('.', '', 1).lstrip('-').isdigit() for val in imu_data):
                        print(f"Error: Datos IMU mal formateados: {imu_data}")
                        continue
                    self.imu_values = list(map(float, imu_data))
                                    
                # Imprime los valores de IR y IMU
                print(f"IR: {self.ir_values} | IMU: {self.imu_values}", end='\r')  
            except Exception as e:
                print(f"Error al leer desde el serial: {e}")
                continue


    def run(self):
        threading.Thread(target=self.read_serial, daemon=True).start()
        try:
            with keyboard.Listener(on_press=self.on_press, on_release=self.on_release) as listener:
                listener.join()
        except KeyboardInterrupt:
            self.ser.close()
            print("\n Serial cerrado")

# ===================== MAIN =====================
if __name__ == "__main__":
    controller = RobotController()
    controller.run()
