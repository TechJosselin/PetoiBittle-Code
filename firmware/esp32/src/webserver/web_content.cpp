#include "web_content.h"

// HTML principal
const char* html_index = R"(<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Bittle Robot Control</title>
    <link rel="stylesheet" href="/styles.css">
</head>
<body>
    <div class="container">
        <header>
            <h1>🤖 Bittle Robot Control</h1>
            <div class="status-bar">
                <span class="status-item">
                    <span class="status-label">Connection:</span>
                    <span id="connection-status" class="status-value">Disconnected</span>
                </span>
                <span class="status-item">
                    <span class="status-label">Battery:</span>
                    <span id="battery-status" class="status-value">--V</span>
                </span>
            </div>
        </header>

        <div class="main-content">
            <!-- Visualisation IK -->
            <div class="panel">
                <h2>Robot Visualization</h2>
                <canvas id="robot-canvas" width="600" height="600"></canvas>
            </div>

            <!-- Contrôles -->
            <div class="panel controls-panel">
                <h2>Controls</h2>
                
                <div class="control-section">
                    <h3>Position Target</h3>
                    <div class="slider-group">
                        <label>
                            X: <span id="target-x-value">0</span> cm
                            <input type="range" id="target-x" min="-15" max="15" step="0.5" value="0">
                        </label>
                        <label>
                            Y: <span id="target-y-value">-12</span> cm
                            <input type="range" id="target-y" min="-20" max="5" step="0.5" value="-12">
                        </label>
                    </div>
                </div>

                <div class="control-section">
                    <h3>Leg Selection</h3>
                    <div class="leg-buttons">
                        <button class="leg-btn active" data-leg="0">Front Left</button>
                        <button class="leg-btn" data-leg="1">Front Right</button>
                        <button class="leg-btn" data-leg="2">Rear Left</button>
                        <button class="leg-btn" data-leg="3">Rear Right</button>
                    </div>
                </div>

                <div class="control-section">
                    <h3>Preset Positions</h3>
                    <div class="preset-buttons">
                        <button class="preset-btn" data-preset="stand">Stand</button>
                        <button class="preset-btn" data-preset="sit">Sit</button>
                        <button class="preset-btn" data-preset="rest">Rest</button>
                        <button class="preset-btn" data-preset="stretch">Stretch</button>
                    </div>
                </div>

                <div class="control-section">
                    <h3>Manual Servo Control</h3>
                    <div class="servo-controls">
                        <div class="servo-group">
                            <label>
                                Shoulder (θ1): <span id="servo0-value">90</span>°
                                <input type="range" id="servo0" min="0" max="180" step="1" value="90">
                            </label>
                            <label>
                                Elbow (θ2): <span id="servo1-value">90</span>°
                                <input type="range" id="servo1" min="0" max="180" step="1" value="90">
                            </label>
                        </div>
                    </div>
                </div>

                <div class="control-section">
                    <h3>Actions</h3>
                    <div class="action-buttons">
                        <button id="btn-calibrate" class="action-btn">Calibrate</button>
                        <button id="btn-center" class="action-btn">Center All</button>
                        <button id="btn-emergency" class="action-btn emergency">Emergency Stop</button>
                    </div>
                </div>
            </div>
        </div>

        <!-- Panneau d'informations -->
        <div class="panel info-panel">
            <h2>Robot Information</h2>
            <div class="info-grid">
                <div class="info-item">
                    <span class="info-label">Selected Leg:</span>
                    <span id="info-leg" class="info-value">Front Left (0)</span>
                </div>
                <div class="info-item">
                    <span class="info-label">Target Position:</span>
                    <span id="info-target" class="info-value">X: 0cm, Y: -12cm</span>
                </div>
                <div class="info-item">
                    <span class="info-label">Calculated Angles:</span>
                    <span id="info-angles" class="info-value">θ1: --°, θ2: --°</span>
                </div>
                <div class="info-item">
                    <span class="info-label">IK Status:</span>
                    <span id="info-ik-status" class="info-value">Valid</span>
                </div>
            </div>
        </div>

        <!-- Console de logs -->
        <div class="panel console-panel">
            <div class="console-header">
                <h2>Console</h2>
                <button id="btn-clear-console" class="btn-clear">Clear</button>
            </div>
            <div id="console" class="console"></div>
        </div>
    </div>

    <script src="/js/config.js"></script>
    <script src="/js/ik.js"></script>
    <script src="/js/net.js"></script>
    <script src="/js/ui.js"></script>
</body>
</html>)";

// CSS
const char* css_styles = R"(:root {
    --primary-color: #2196F3;
    --secondary-color: #4CAF50;
    --danger-color: #f44336;
    --warning-color: #ff9800;
    --bg-color: #1a1a2e;
    --panel-bg: #16213e;
    --text-color: #e0e0e0;
    --border-color: #0f3460;
}

* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background: var(--bg-color);
    color: var(--text-color);
    line-height: 1.6;
}

.container {
    max-width: 1400px;
    margin: 0 auto;
    padding: 20px;
}

header {
    background: var(--panel-bg);
    padding: 20px;
    border-radius: 10px;
    margin-bottom: 20px;
    border: 2px solid var(--border-color);
}

h1 {
    color: var(--primary-color);
    margin-bottom: 15px;
}

h2 {
    color: var(--primary-color);
    margin-bottom: 15px;
    font-size: 1.5em;
}

h3 {
    color: var(--secondary-color);
    margin-bottom: 10px;
    font-size: 1.2em;
}

.status-bar {
    display: flex;
    gap: 30px;
    flex-wrap: wrap;
}

.status-item {
    display: flex;
    gap: 10px;
}

.status-label {
    color: #888;
}

.status-value {
    color: var(--text-color);
    font-weight: bold;
}

#connection-status.connected {
    color: var(--secondary-color);
}

#connection-status.disconnected {
    color: var(--danger-color);
}

.main-content {
    display: grid;
    grid-template-columns: 2fr 1fr;
    gap: 20px;
    margin-bottom: 20px;
}

.panel {
    background: var(--panel-bg);
    padding: 20px;
    border-radius: 10px;
    border: 2px solid var(--border-color);
}

#robot-canvas {
    display: block;
    width: 100%;
    max-width: 600px;
    height: auto;
    background: #0a0e27;
    border-radius: 8px;
    border: 2px solid var(--border-color);
}

.controls-panel {
    display: flex;
    flex-direction: column;
    gap: 20px;
}

.control-section {
    padding: 15px;
    background: rgba(15, 52, 96, 0.3);
    border-radius: 8px;
    border: 1px solid var(--border-color);
}

.slider-group {
    display: flex;
    flex-direction: column;
    gap: 15px;
}

.slider-group label {
    display: flex;
    flex-direction: column;
    gap: 8px;
}

input[type="range"] {
    width: 100%;
    height: 8px;
    background: var(--border-color);
    border-radius: 4px;
    outline: none;
    -webkit-appearance: none;
}

input[type="range"]::-webkit-slider-thumb {
    -webkit-appearance: none;
    width: 20px;
    height: 20px;
    background: var(--primary-color);
    border-radius: 50%;
    cursor: pointer;
}

input[type="range"]::-moz-range-thumb {
    width: 20px;
    height: 20px;
    background: var(--primary-color);
    border-radius: 50%;
    cursor: pointer;
    border: none;
}

.leg-buttons,
.preset-buttons,
.action-buttons {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: 10px;
}

button {
    padding: 12px 20px;
    font-size: 14px;
    font-weight: 600;
    border: 2px solid var(--border-color);
    border-radius: 6px;
    cursor: pointer;
    transition: all 0.3s ease;
    background: var(--panel-bg);
    color: var(--text-color);
}

button:hover {
    transform: translateY(-2px);
    box-shadow: 0 4px 8px rgba(33, 150, 243, 0.3);
}

button:active {
    transform: translateY(0);
}

.leg-btn.active {
    background: var(--primary-color);
    color: white;
    border-color: var(--primary-color);
}

.preset-btn {
    background: linear-gradient(135deg, var(--primary-color), #1976D2);
    border-color: var(--primary-color);
}

.action-btn {
    background: linear-gradient(135deg, var(--secondary-color), #388E3C);
    border-color: var(--secondary-color);
}

.action-btn.emergency {
    background: linear-gradient(135deg, var(--danger-color), #c62828);
    border-color: var(--danger-color);
}

.servo-controls {
    display: flex;
    flex-direction: column;
    gap: 15px;
}

.servo-group {
    display: flex;
    flex-direction: column;
    gap: 15px;
}

.servo-group label {
    display: flex;
    flex-direction: column;
    gap: 8px;
}

.info-panel {
    grid-column: 1 / -1;
}

.info-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
    gap: 15px;
}

.info-item {
    padding: 15px;
    background: rgba(15, 52, 96, 0.3);
    border-radius: 6px;
    border: 1px solid var(--border-color);
}

.info-label {
    color: #888;
    font-size: 0.9em;
    display: block;
    margin-bottom: 5px;
}

.info-value {
    color: var(--text-color);
    font-weight: bold;
    font-size: 1.1em;
}

.console-panel {
    grid-column: 1 / -1;
}

.console-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 15px;
}

.btn-clear {
    padding: 8px 16px;
    font-size: 12px;
    background: var(--danger-color);
    border-color: var(--danger-color);
}

.console {
    background: #0a0e27;
    padding: 15px;
    border-radius: 6px;
    border: 2px solid var(--border-color);
    max-height: 200px;
    overflow-y: auto;
    font-family: 'Courier New', monospace;
    font-size: 13px;
}

.console-entry {
    padding: 4px 0;
    border-bottom: 1px solid rgba(15, 52, 96, 0.5);
}

.console-entry:last-child {
    border-bottom: none;
}

.console-time {
    color: #888;
    margin-right: 10px;
}

.console-message {
    color: var(--text-color);
}

.console-entry.error .console-message {
    color: var(--danger-color);
}

.console-entry.warning .console-message {
    color: var(--warning-color);
}

.console-entry.success .console-message {
    color: var(--secondary-color);
}

::-webkit-scrollbar {
    width: 10px;
}

::-webkit-scrollbar-track {
    background: var(--panel-bg);
}

::-webkit-scrollbar-thumb {
    background: var(--border-color);
    border-radius: 5px;
}

::-webkit-scrollbar-thumb:hover {
    background: var(--primary-color);
}

@media (max-width: 1024px) {
    .main-content {
        grid-template-columns: 1fr;
    }
    
    .info-grid {
        grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    }
}

@media (max-width: 768px) {
    .container {
        padding: 10px;
    }
    
    header {
        padding: 15px;
    }
    
    h1 {
        font-size: 1.5em;
    }
    
    .leg-buttons,
    .preset-buttons,
    .action-buttons {
        grid-template-columns: 1fr;
    }
    
    .info-grid {
        grid-template-columns: 1fr;
    }
})";

// JavaScript - config.js
const char* js_config = R"(// Configuration du robot Bittle
const CONFIG = {
    // Dimensions des segments de patte (en cm)
    LEG: {
        L1: 7.0,  // Longueur du premier segment (épaule -> coude)
        L2: 7.2   // Longueur du second segment (coude -> pied)
    },
    
    // Limites des servos (en degrés)
    SERVO_LIMITS: {
        MIN: 0,
        MAX: 180,
        CENTER: 90
    },
    
    // Configuration des pattes
    LEGS: [
        { name: 'Front Left', id: 0, servos: [0, 1] },
        { name: 'Front Right', id: 1, servos: [2, 3] },
        { name: 'Rear Left', id: 2, servos: [4, 5] },
        { name: 'Rear Right', id: 3, servos: [6, 7] }
    ],
    
    // Positions prédéfinies
    PRESETS: {
        stand: { x: 0, y: -12 },
        sit: { x: 0, y: -8 },
        rest: { x: 0, y: -5 },
        stretch: { x: 10, y: -15 }
    },
    
    // Configuration réseau
    NETWORK: {
        UPDATE_INTERVAL: 100,  // ms
        TIMEOUT: 5000,         // ms
        RETRY_DELAY: 1000      // ms
    },
    
    // Configuration du canvas
    CANVAS: {
        SCALE: 20,  // Pixels par cm
        ORIGIN_X: 300,
        ORIGIN_Y: 300
    }
};)";

// JavaScript - ik.js
const char* js_ik = R"(// Module de calcul de cinématique inverse (IK)
class InverseKinematics {
    constructor(l1, l2) {
        this.l1 = l1;  // Longueur segment 1
        this.l2 = l2;  // Longueur segment 2
    }
    
    // Calcule les angles des servos pour atteindre la position (x, y)
    calculate(x, y) {
        const distance = Math.sqrt(x * x + y * y);
        
        // Vérifier si la position est atteignable
        if (distance > (this.l1 + this.l2) || distance < Math.abs(this.l1 - this.l2)) {
            return { valid: false, theta1: 0, theta2: 0 };
        }
        
        // Calcul de theta2 (angle du coude)
        const cosTheta2 = (x * x + y * y - this.l1 * this.l1 - this.l2 * this.l2) / 
                          (2 * this.l1 * this.l2);
        const theta2 = Math.acos(Math.max(-1, Math.min(1, cosTheta2)));
        
        // Calcul de theta1 (angle de l'épaule)
        const k1 = this.l1 + this.l2 * Math.cos(theta2);
        const k2 = this.l2 * Math.sin(theta2);
        const theta1 = Math.atan2(y, x) - Math.atan2(k2, k1);
        
        // Convertir en degrés et ajuster pour les servos
        let servo1 = 90 - (theta1 * 180 / Math.PI);
        let servo2 = 90 - (theta2 * 180 / Math.PI);
        
        // Limiter aux plages valides des servos
        servo1 = Math.max(0, Math.min(180, servo1));
        servo2 = Math.max(0, Math.min(180, servo2));
        
        return {
            valid: true,
            theta1: theta1 * 180 / Math.PI,
            theta2: theta2 * 180 / Math.PI,
            servo1: Math.round(servo1),
            servo2: Math.round(servo2)
        };
    }
    
    // Calcule la position (x, y) à partir des angles des servos
    forward(servo1Angle, servo2Angle) {
        // Convertir les angles servo en radians
        const theta1 = (90 - servo1Angle) * Math.PI / 180;
        const theta2 = (90 - servo2Angle) * Math.PI / 180;
        
        // Calcul de la position
        const x = this.l1 * Math.cos(theta1) + this.l2 * Math.cos(theta1 + theta2);
        const y = this.l1 * Math.sin(theta1) + this.l2 * Math.sin(theta1 + theta2);
        
        return { x, y };
    }
}

// Instance globale de l'IK
const ik = new InverseKinematics(CONFIG.LEG.L1, CONFIG.LEG.L2);)";

// JavaScript - net.js
const char* js_net = R"(// Module de communication réseau
class NetworkManager {
    constructor() {
        this.connected = false;
        this.statusCheckInterval = null;
    }
    
    async init() {
        // Démarrer la vérification du statut
        this.startStatusCheck();
        return true;
    }
    
    startStatusCheck() {
        this.statusCheckInterval = setInterval(async () => {
            try {
                const status = await this.getStatus();
                this.connected = true;
                this.updateConnectionStatus(true);
                
                // Mettre à jour l'affichage du statut
                if (status.battery) {
                    document.getElementById('battery-status').textContent = 
                        status.battery.toFixed(2) + 'V';
                }
            } catch (error) {
                this.connected = false;
                this.updateConnectionStatus(false);
            }
        }, CONFIG.NETWORK.UPDATE_INTERVAL);
    }
    
    updateConnectionStatus(connected) {
        const statusElement = document.getElementById('connection-status');
        if (connected) {
            statusElement.textContent = 'Connected';
            statusElement.className = 'status-value connected';
        } else {
            statusElement.textContent = 'Disconnected';
            statusElement.className = 'status-value disconnected';
        }
    }
    
    async getStatus() {
        const response = await fetch('/api/status');
        if (!response.ok) throw new Error('Status check failed');
        return await response.json();
    }
    
    async sendCommand(command, params = {}) {
        try {
            const response = await fetch('/api/command', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ command, ...params })
            });
            
            if (!response.ok) throw new Error('Command failed');
            return await response.json();
        } catch (error) {
            console.error('Command error:', error);
            throw error;
        }
    }
    
    async setServo(channel, angle) {
        try {
            const response = await fetch('/api/servo', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ channel, angle })
            });
            
            if (!response.ok) throw new Error('Servo command failed');
            return await response.json();
        } catch (error) {
            console.error('Servo error:', error);
            throw error;
        }
    }
    
    destroy() {
        if (this.statusCheckInterval) {
            clearInterval(this.statusCheckInterval);
        }
    }
}

// Instance globale du gestionnaire réseau
const network = new NetworkManager();)";

// JavaScript - ui.js
const char* js_ui = R"(// Module d'interface utilisateur
class UIManager {
    constructor() {
        this.canvas = document.getElementById('robot-canvas');
        this.ctx = this.canvas.getContext('2d');
        this.selectedLeg = 0;
        this.targetX = 0;
        this.targetY = -12;
        this.currentAngles = { theta1: 0, theta2: 0, servo1: 90, servo2: 90 };
    }
    
    init() {
        this.setupEventListeners();
        this.updateDisplay();
        this.draw();
        this.log('UI initialized', 'success');
    }
    
    setupEventListeners() {
        // Sliders de position cible
        document.getElementById('target-x').addEventListener('input', (e) => {
            this.targetX = parseFloat(e.target.value);
            document.getElementById('target-x-value').textContent = this.targetX;
            this.updateIK();
        });
        
        document.getElementById('target-y').addEventListener('input', (e) => {
            this.targetY = parseFloat(e.target.value);
            document.getElementById('target-y-value').textContent = this.targetY;
            this.updateIK();
        });
        
        // Boutons de sélection de patte
        document.querySelectorAll('.leg-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                document.querySelectorAll('.leg-btn').forEach(b => b.classList.remove('active'));
                e.target.classList.add('active');
                this.selectedLeg = parseInt(e.target.dataset.leg);
                this.updateDisplay();
                this.log(`Selected leg: ${CONFIG.LEGS[this.selectedLeg].name}`, 'success');
            });
        });
        
        // Boutons de positions prédéfinies
        document.querySelectorAll('.preset-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const preset = e.target.dataset.preset;
                const pos = CONFIG.PRESETS[preset];
                document.getElementById('target-x').value = pos.x;
                document.getElementById('target-y').value = pos.y;
                this.targetX = pos.x;
                this.targetY = pos.y;
                document.getElementById('target-x-value').textContent = pos.x;
                document.getElementById('target-y-value').textContent = pos.y;
                this.updateIK();
                this.log(`Preset applied: ${preset}`, 'success');
            });
        });
        
        // Sliders de contrôle manuel des servos
        document.getElementById('servo0').addEventListener('input', (e) => {
            const angle = parseInt(e.target.value);
            document.getElementById('servo0-value').textContent = angle;
            this.manualServoControl(0, angle);
        });
        
        document.getElementById('servo1').addEventListener('input', (e) => {
            const angle = parseInt(e.target.value);
            document.getElementById('servo1-value').textContent = angle;
            this.manualServoControl(1, angle);
        });
        
        // Boutons d'action
        document.getElementById('btn-calibrate').addEventListener('click', () => {
            this.calibrate();
        });
        
        document.getElementById('btn-center').addEventListener('click', () => {
            this.centerAll();
        });
        
        document.getElementById('btn-emergency').addEventListener('click', () => {
            this.emergencyStop();
        });
        
        document.getElementById('btn-clear-console').addEventListener('click', () => {
            document.getElementById('console').innerHTML = '';
        });
    }
    
    updateIK() {
        const result = ik.calculate(this.targetX, this.targetY);
        
        if (result.valid) {
            this.currentAngles = result;
            this.sendServoCommands(result.servo1, result.servo2);
            document.getElementById('info-ik-status').textContent = 'Valid';
            document.getElementById('info-ik-status').style.color = 'var(--secondary-color)';
        } else {
            document.getElementById('info-ik-status').textContent = 'Out of reach';
            document.getElementById('info-ik-status').style.color = 'var(--danger-color)';
            this.log('Target position out of reach', 'warning');
        }
        
        this.updateDisplay();
        this.draw();
    }
    
    manualServoControl(servoIndex, angle) {
        const leg = CONFIG.LEGS[this.selectedLeg];
        const servoChannel = leg.servos[servoIndex];
        
        network.setServo(servoChannel, angle)
            .then(() => {
                this.log(`Servo ${servoChannel} set to ${angle}°`, 'success');
            })
            .catch(error => {
                this.log(`Failed to set servo: ${error.message}`, 'error');
            });
        
        // Calculer la position résultante
        const servo1 = parseInt(document.getElementById('servo0').value);
        const servo2 = parseInt(document.getElementById('servo1').value);
        const pos = ik.forward(servo1, servo2);
        
        this.targetX = pos.x;
        this.targetY = pos.y;
        document.getElementById('target-x').value = pos.x;
        document.getElementById('target-y').value = pos.y;
        document.getElementById('target-x-value').textContent = pos.x.toFixed(1);
        document.getElementById('target-y-value').textContent = pos.y.toFixed(1);
        
        this.draw();
    }
    
    sendServoCommands(angle1, angle2) {
        const leg = CONFIG.LEGS[this.selectedLeg];
        
        Promise.all([
            network.setServo(leg.servos[0], angle1),
            network.setServo(leg.servos[1], angle2)
        ])
        .then(() => {
            this.log(`Servos updated: θ1=${angle1}°, θ2=${angle2}°`, 'success');
        })
        .catch(error => {
            this.log(`Failed to update servos: ${error.message}`, 'error');
        });
    }
    
    calibrate() {
        this.log('Starting calibration...', 'success');
        network.sendCommand('calibrate')
            .then(() => {
                this.log('Calibration complete', 'success');
            })
            .catch(error => {
                this.log(`Calibration failed: ${error.message}`, 'error');
            });
    }
    
    centerAll() {
        this.log('Centering all servos...', 'success');
        network.sendCommand('center')
            .then(() => {
                this.log('All servos centered', 'success');
            })
            .catch(error => {
                this.log(`Center failed: ${error.message}`, 'error');
            });
    }
    
    emergencyStop() {
        this.log('EMERGENCY STOP', 'error');
        network.sendCommand('stop')
            .then(() => {
                this.log('Robot stopped', 'success');
            })
            .catch(error => {
                this.log(`Stop failed: ${error.message}`, 'error');
            });
    }
    
    updateDisplay() {
        const leg = CONFIG.LEGS[this.selectedLeg];
        document.getElementById('info-leg').textContent = `${leg.name} (${leg.id})`;
        document.getElementById('info-target').textContent = 
            `X: ${this.targetX.toFixed(1)}cm, Y: ${this.targetY.toFixed(1)}cm`;
        
        if (this.currentAngles.valid) {
            document.getElementById('info-angles').textContent = 
                `θ1: ${this.currentAngles.servo1}°, θ2: ${this.currentAngles.servo2}°`;
        }
    }
    
    draw() {
        const ctx = this.ctx;
        const canvas = this.canvas;
        
        // Effacer le canvas
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        
        // Paramètres de dessin
        const scale = CONFIG.CANVAS.SCALE;
        const originX = CONFIG.CANVAS.ORIGIN_X;
        const originY = CONFIG.CANVAS.ORIGIN_Y;
        
        // Dessiner la grille
        this.drawGrid(ctx, scale, originX, originY);
        
        // Dessiner le système de coordonnées
        this.drawAxes(ctx, originX, originY);
        
        // Dessiner la patte
        this.drawLeg(ctx, scale, originX, originY);
        
        // Dessiner la cible
        this.drawTarget(ctx, scale, originX, originY);
    }
    
    drawGrid(ctx, scale, originX, originY) {
        ctx.strokeStyle = '#0f3460';
        ctx.lineWidth = 1;
        
        // Lignes verticales
        for (let x = -30; x <= 30; x += 5) {
            const px = originX + x * scale;
            ctx.beginPath();
            ctx.moveTo(px, 0);
            ctx.lineTo(px, this.canvas.height);
            ctx.stroke();
        }
        
        // Lignes horizontales
        for (let y = -30; y <= 30; y += 5) {
            const py = originY - y * scale;
            ctx.beginPath();
            ctx.moveTo(0, py);
            ctx.lineTo(this.canvas.width, py);
            ctx.stroke();
        }
    }
    
    drawAxes(ctx, originX, originY) {
        ctx.strokeStyle = '#2196F3';
        ctx.lineWidth = 2;
        
        // Axe X
        ctx.beginPath();
        ctx.moveTo(0, originY);
        ctx.lineTo(this.canvas.width, originY);
        ctx.stroke();
        
        // Axe Y
        ctx.beginPath();
        ctx.moveTo(originX, 0);
        ctx.lineTo(originX, this.canvas.height);
        ctx.stroke();
    }
    
    drawLeg(ctx, scale, originX, originY) {
        if (!this.currentAngles.valid) return;
        
        const theta1 = (90 - this.currentAngles.servo1) * Math.PI / 180;
        const theta2 = (90 - this.currentAngles.servo2) * Math.PI / 180;
        
        // Position du coude
        const elbowX = CONFIG.LEG.L1 * Math.cos(theta1);
        const elbowY = CONFIG.LEG.L1 * Math.sin(theta1);
        
        // Position du pied
        const footX = elbowX + CONFIG.LEG.L2 * Math.cos(theta1 + theta2);
        const footY = elbowY + CONFIG.LEG.L2 * Math.sin(theta1 + theta2);
        
        // Dessiner le premier segment
        ctx.strokeStyle = '#4CAF50';
        ctx.lineWidth = 4;
        ctx.beginPath();
        ctx.moveTo(originX, originY);
        ctx.lineTo(originX + elbowX * scale, originY - elbowY * scale);
        ctx.stroke();
        
        // Dessiner le second segment
        ctx.strokeStyle = '#2196F3';
        ctx.beginPath();
        ctx.moveTo(originX + elbowX * scale, originY - elbowY * scale);
        ctx.lineTo(originX + footX * scale, originY - footY * scale);
        ctx.stroke();
        
        // Dessiner les articulations
        this.drawJoint(ctx, originX, originY, '#ff9800');
        this.drawJoint(ctx, originX + elbowX * scale, originY - elbowY * scale, '#ff9800');
        this.drawJoint(ctx, originX + footX * scale, originY - footY * scale, '#4CAF50');
    }
    
    drawJoint(ctx, x, y, color) {
        ctx.fillStyle = color;
        ctx.beginPath();
        ctx.arc(x, y, 6, 0, 2 * Math.PI);
        ctx.fill();
    }
    
    drawTarget(ctx, scale, originX, originY) {
        const px = originX + this.targetX * scale;
        const py = originY - this.targetY * scale;
        
        ctx.strokeStyle = '#f44336';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.arc(px, py, 8, 0, 2 * Math.PI);
        ctx.stroke();
        
        // Croix
        ctx.beginPath();
        ctx.moveTo(px - 12, py);
        ctx.lineTo(px + 12, py);
        ctx.moveTo(px, py - 12);
        ctx.lineTo(px, py + 12);
        ctx.stroke();
    }
    
    log(message, type = 'info') {
        const console = document.getElementById('console');
        const entry = document.createElement('div');
        entry.className = `console-entry ${type}`;
        
        const time = new Date().toLocaleTimeString();
        entry.innerHTML = `
            <span class="console-time">[${time}]</span>
            <span class="console-message">${message}</span>
        `;
        
        console.appendChild(entry);
        console.scrollTop = console.scrollHeight;
    }
}

// Initialisation au chargement de la page
document.addEventListener('DOMContentLoaded', async () => {
    const ui = new UIManager();
    ui.init();
    
    await network.init();
    ui.log('System ready', 'success');
});)";
