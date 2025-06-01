from flask import Flask, request, jsonify
from flask_jwt_extended import (
    JWTManager, create_access_token, jwt_required,
    get_jwt_identity, get_jwt
)
from datetime import timedelta

app = Flask(__name__)

# Secretul pentru semnarea token-ului JWT
app.config["JWT_SECRET_KEY"] = "cheia_ta_secreta_123"
app.config["JWT_ACCESS_TOKEN_EXPIRES"] = timedelta(hours=1)

jwt = JWTManager(app)

# Tokenuri invalidate (logout)
token_blacklist = set()

# Utilizatori hardcodați
users = {
    "user1": {"password": "parola1", "role": "admin"},
    "user2": {"password": "parola2", "role": "owner"},
    "user3": {"password": "parolaX", "role": "owner"}
}

@app.route('/')
def home():
    return "Bine ai venit la API-ul securizat!"

# ================================
# AUTHENTIFICARE / LOGIN
# ================================
@app.route("/auth", methods=["POST"])
def login():
    data = request.get_json()
    username = data.get("username")
    password = data.get("password")

    user = users.get(username)
    if user and user["password"] == password:
        access_token = create_access_token(identity={"username": username, "role": user["role"]})
        return jsonify(access_token=access_token), 200
    return jsonify(msg="Credențiale invalide!"), 401

# ================================
# VALIDARE TOKEN
# ================================
@app.route("/auth/jwtStore", methods=["GET"])
@jwt_required()
def validate_token():
    jti = get_jwt()["jti"]
    if jti in token_blacklist:
        return jsonify(msg="Token invalidat."), 404

    identity = get_jwt_identity()
    return jsonify(valid=True, role=identity["role"]), 200

# ================================
# LOGOUT
# ================================
@app.route("/auth/jwtStore", methods=["DELETE"])
@jwt_required()
def logout():
    jti = get_jwt()["jti"]
    token_blacklist.add(jti)
    return jsonify(msg="Logout realizat cu succes."), 200

# ================================
# API protejat (EXEMPLU)
# ================================
@app.route("/sensor/data", methods=["GET"])
@jwt_required()
def read_sensor_data():
    identity = get_jwt_identity()
    role = identity["role"]

    if role in ["owner", "admin"]:
        return jsonify(data="Temperatura: 25.4°C"), 200
    return jsonify(msg="Acces interzis!"), 403

@app.route("/sensor/config", methods=["POST"])
@jwt_required()
def modify_config():
    identity = get_jwt_identity()
    role = identity["role"]

    if role == "admin":
        return jsonify(msg="Configurație actualizată!"), 200
    return jsonify(msg="Doar adminul poate modifica configurația!"), 403

if __name__ == "__main__":
    app.run(debug=True)
