import serial
import time
from pynput import keyboard

# ===================== CONFIGURACIÓN =====================
speed = 255 # Velocidad inicial
key_pressed = None
movement_active = False

# Conecta al puerto serial
ser = serial.Serial('/dev/ttyUSB0', 115200)
print("Serial port opened en /dev/ttyUSB0")

# ===================== AYUDA DE TECLAS =====================
def print_controls():
    print("""
  CONTROLES DEL ROBOT MECANUM
=======================================
Movimiento:
    W  →  Avanzar
    S  →  Retroceder
    A  →  Izquierda (giro tipo tanque)
    D  →  Derecha  (giro tipo tanque)
    Q  →  Rotación antihoraria
    E  →  Rotación horaria
    Z  →  Diagonal Izquierda Adelante
    C  →  Diagonal Derecha Adelante
    X  →  Detenerse

Velocidad:
    +  →  Aumentar velocidad
    -  →  Disminuir velocidad

Presiona ESC para salir
=======================================
""")

# ===================== FUNCIONES =====================
def get_movement(key, speed):
    if key == 'w': return [speed, speed, speed, speed]
    if key == 's': return [-speed, -speed, -speed, -speed]
    if key == 'a': return [-speed, speed, speed, -speed]
    if key == 'd': return [speed, -speed, -speed, speed]
    if key == 'q': return [-speed, -speed, speed, speed]
    if key == 'e': return [speed, speed, -speed, -speed]
    if key == 'z': return [0, speed, speed, 0]
    if key == 'c': return [speed, 0, 0, speed]
    if key == 'x': return [0, 0, 0, 0]
    return None

def send_array(ser, array):
    array_str = ','.join(map(str, array)) + '\n'
    ser.write(array_str.encode())
    print(f"Sent: {array}")

def on_press(key):
    global key_pressed, speed, movement_active

    try:
        k = key.char

        if k == '+':
            speed = min(255, speed + 10)
            print(f" Velocidad aumentada: {speed}")
        elif k == '-':
            speed = max(0, speed - 10)
            print(f" Velocidad reducida: {speed}")
        else:
            movement = get_movement(k, speed)
            if movement:
                key_pressed = k
                send_array(ser, movement)
                movement_active = True

    except AttributeError:
        # ESC para salir
        if key == keyboard.Key.esc:
            print("\n Terminando...")
            ser.close()
            return False

def on_release(key):
    global movement_active
    if movement_active:
        send_array(ser, [0, 0, 0, 0])
        movement_active = False

# ===================== MAIN =====================
if __name__ == "__main__":
    print_controls()

    try:
        with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
            listener.join()

    except KeyboardInterrupt:
        ser.close()
        print("\n💤 Serial port closed")
