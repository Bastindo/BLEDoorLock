import socket
from datetime import datetime

# Konfigurationsvariablen
HOST = '127.0.0.1'  # localhost
PORT = 64000        # Port zum Lauschen
LOG_FILE = 'server_log.txt'  # Pfad zur Logdatei

# Funktion zum Schreiben von Nachrichten mit Zeitstempel in die Datei
def log_message(message, log_file):
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    with open(log_file, 'a') as file:
        file.write(f"[{timestamp}] {message}\n")

# Erstelle einen TCP-Server mit IPv4-Adresse
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    server.bind((HOST, PORT))
    server.listen()
    print(f"Server läuft und wartet auf Verbindungen auf {HOST}:{PORT}...")

    while True:
        conn, addr = server.accept()
        with conn:
            print(f"Verbindung von {addr} hergestellt.")
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                message = data.decode('utf-8')
                print(f"Nachricht erhalten: {message}")
                log_message(message, LOG_FILE)
                conn.sendall(b'Nachricht empfangen')
