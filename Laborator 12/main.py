from flask import Flask, request, jsonify

app = Flask(__name__)

# Simulăm o "bază de date" simplă cu o listă
devices = []

@app.route('/')
def index():
    return "<h3>API - Lista Dispozitivelor IoT</h3>"

@app.route('/devices', methods=['GET'])
def get_devices():
    return jsonify(devices), 200

@app.route('/devices/<int:device_id>', methods=['GET'])
def get_device(device_id):
    for device in devices:
        if device['id'] == device_id:
            return jsonify(device), 200
    return jsonify({'error': 'Device not found'}), 404

@app.route('/devices', methods=['POST'])
def add_device():
    data = request.json
    device = {
        'id': data['id'],
        'name': data['name'],
        'status': data.get('status', 'offline')
    }
    devices.append(device)
    return jsonify({'message': 'Device added'}), 201

@app.route('/devices/<int:device_id>', methods=['PUT'])
def update_device(device_id):
    data = request.json
    for device in devices:
        if device['id'] == device_id:
            device['name'] = data.get('name', device['name'])
            device['status'] = data.get('status', device['status'])
            return jsonify({'message': 'Device updated'}), 200
    return jsonify({'error': 'Device not found'}), 404

@app.route('/devices/<int:device_id>', methods=['DELETE'])
def delete_device(device_id):
    for device in devices:
        if device['id'] == device_id:
            devices.remove(device)
            return jsonify({'message': 'Device deleted'}), 200
    return jsonify({'error': 'Device not found'}), 404

if __name__ == '__main__':
    app.run(debug=True)
