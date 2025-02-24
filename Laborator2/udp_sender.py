import socket
import time

PEER_IP = "192.168.89.49"
PEER_PORT = 10001
GPIO_KEY = "GPIO4"
INITIAL_GPIO_VALUE = "0"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
counter = 0

while True:
    try:
        gpio_value = "1" if counter % 2 == 0 else "0"
        message = f"{GPIO_KEY}={gpio_value}".encode('ascii')
        sock.sendto(message, (PEER_IP, PEER_PORT))
        print(f"Sent message: {message.decode('ascii')}")
        time.sleep(1)
        counter += 1
    except KeyboardInterrupt:
        print("Program interrupted by user.")
        break
    except Exception as e:
        print(f"An error occurred: {e}")
        break

sock.close()
