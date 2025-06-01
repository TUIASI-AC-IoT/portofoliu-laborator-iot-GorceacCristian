from flask import Flask, request, jsonify
import os

app = Flask(__name__)
BASE_DIR = "files"

if not os.path.exists(BASE_DIR):
    os.makedirs(BASE_DIR)

@app.route('/files', methods=['GET'])
def list_files():
    return jsonify(os.listdir(BASE_DIR)), 200

@app.route('/files/<filename>', methods=['GET'])
def read_file(filename):
    filepath = os.path.join(BASE_DIR, filename)
    if not os.path.exists(filepath):
        return jsonify({'error': 'File not found'}), 404
    with open(filepath, 'r') as f:
        content = f.read()
    return jsonify({'filename': filename, 'content': content}), 200

@app.route('/files', methods=['POST'])
def create_file():
    data = request.json
    filename = data.get('filename')
    content = data.get('content', '')
    if not filename:
        return jsonify({'error': 'Filename is required'}), 400
    filepath = os.path.join(BASE_DIR, filename)
    with open(filepath, 'w') as f:
        f.write(content)
    return jsonify({'message': 'File created', 'filename': filename}), 201

@app.route('/files/content-only', methods=['POST'])
def create_file_with_random_name():
    import uuid
    filename = f"{uuid.uuid4().hex}.txt"
    content = request.json.get('content', '')
    filepath = os.path.join(BASE_DIR, filename)
    with open(filepath, 'w') as f:
        f.write(content)
    return jsonify({'message': 'File created', 'filename': filename}), 201

@app.route('/files/<filename>', methods=['DELETE'])
def delete_file(filename):
    filepath = os.path.join(BASE_DIR, filename)
    if not os.path.exists(filepath):
        return jsonify({'error': 'File not found'}), 404
    os.remove(filepath)
    return jsonify({'message': f'File {filename} deleted'}), 200

@app.route('/files/<filename>', methods=['PUT'])
def update_file(filename):
    filepath = os.path.join(BASE_DIR, filename)
    if not os.path.exists(filepath):
        return jsonify({'error': 'File not found'}), 404
    content = request.json.get('content', '')
    with open(filepath, 'w') as f:
        f.write(content)
    return jsonify({'message': f'File {filename} updated'}), 200

if __name__ == '__main__':
    app.run(debug=True)
