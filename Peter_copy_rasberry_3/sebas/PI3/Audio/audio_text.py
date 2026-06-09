import socket

SERVER_PORT = 8001  # 🔹 Puerto donde escucha el servidor

# 🔹 Crear socket UDP
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", SERVER_PORT))  # Escuchar en cualquier IP

print(f"📡 Servidor UDP esperando clientes en el puerto {SERVER_PORT}...")

# 🔹 Esperar mensaje de conexión del cliente
buffer, client_addr = sock.recvfrom(1024)  # Recibir hasta 1024 bytes
print(f"📨 Cliente conectado desde: {client_addr[0]}")

# 🔹 Mensaje a enviar al cliente
message = "2, Eso que veo es un..."

# 🔹 Enviar mensaje al cliente
sock.sendto(message.encode(), client_addr)
print("✅ Mensaje enviado al cliente.")

# 🔹 Esperar confirmación del cliente
print("📡 Esperando confirmación del cliente...")
confirmation, client_addr = sock.recvfrom(1024)
print(f"✅ Confirmación recibida: {confirmation.decode()}")

message = "1, Eso que veo es un..."
# 🔹 Enviar mensaje al cliente
sock.sendto(message.encode(), client_addr)
print("✅ Mensaje enviado al cliente.")

#🔹 Cerrar el socket
sock.close()
