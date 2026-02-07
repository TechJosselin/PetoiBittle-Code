#include "web_content.h"

// HTML principal
const char* html_index = R"(<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🤖 Bittle Robot IK Control</title>
    <style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:linear-gradient(135deg,#667eea,#764ba2);color:#333;min-height:100vh;padding:20px}
.container{max-width:1400px;margin:0 auto;background:#fff;border-radius:12px;box-shadow:0 20px 60px rgba(0,0,0,.3);overflow:hidden}
.header{background:linear-gradient(135deg,#667eea,#764ba2);color:#fff;padding:30px;border-bottom:4px solid #fa0}
.header h1{font-size:2.2em;margin-bottom:15px;text-shadow:2px 2px 4px rgba(0,0,0,.3)}
.status-bar{display:flex;gap:30px;flex-wrap:wrap;font-size:.95em}
.status-item{background:rgba(255,255,255,.2);padding:8px 12px;border-radius:6px;border-left:3px solid #fa0}
.main-layout{display:grid;grid-template-columns:1.5fr 1fr;gap:30px;padding:30px;min-height:600px}
.ik-section{display:flex;flex-direction:column;gap:15px}
.section-header{display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:20px}
.section-header h2{font-size:1.5em;color:#667eea}
.controls-inline{display:flex;gap:15px;align-items:center}
.controls-inline label{font-weight:500;color:#555}
.controls-inline select{padding:6px 10px;border:2px solid #ddd;border-radius:4px;font-size:1em;cursor:pointer;transition:border-color .3s}
.controls-inline select:hover,.controls-inline select:focus{border-color:#667eea;outline:none}
input[type="checkbox"]{width:18px;height:18px;cursor:pointer;accent-color:#667eea}
.canvas-container{flex:1;display:flex;align-items:center;justify-content:center;background:#f9f9f9;border:2px solid #e0e0e0;border-radius:8px;overflow:hidden;min-height:500px}
#ikCanvas{display:block;width:100%;height:100%;cursor:grab}
#ikCanvas:active{cursor:grabbing}
.canvas-info{background:#f0f4ff;padding:12px;border-radius:6px;font-size:.9em;color:#555;border-left:4px solid #667eea}
.canvas-info p{margin:5px 0}
.config-section,.simulation-section{background:#f9f9f9;padding:20px;border-radius:8px;border:2px solid #e0e0e0}
.config-section h2,.simulation-section h2{font-size:1.3em;color:#667eea;margin-bottom:20px;padding-bottom:10px;border-bottom:2px solid #ddd}
.servo-config-panel{background:#fff;padding:15px;border-radius:6px;margin-bottom:15px;border:1px solid #ddd}
.servo-config-panel h3{color:#764ba2;margin-bottom:12px;font-size:1.1em}
.config-grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-bottom:10px}
.config-input{display:flex;flex-direction:column;gap:5px}
.config-input label{font-size:.9em;font-weight:500;color:#555}
.config-input input[type="number"]{padding:6px 8px;border:1px solid #ddd;border-radius:4px;font-size:.95em;transition:border-color .3s}
.config-input input[type="number"]:focus{border-color:#667eea;outline:none;box-shadow:0 0 0 3px rgba(102,126,234,.1)}
.servo-output{background:#f0f4ff;padding:8px 12px;border-radius:4px;font-weight:600;color:#667eea;font-size:.95em;border-left:3px solid #667eea}
.servo-output span{font-size:1.1em}
.button-group{display:flex;gap:10px;margin-top:20px}
.btn{flex:1;padding:12px 20px;border:none;border-radius:6px;font-size:1em;font-weight:600;cursor:pointer;transition:all .3s;text-align:center;min-height:44px}
.btn-primary{background:linear-gradient(135deg,#667eea,#764ba2);color:#fff}
.btn-primary:hover{transform:translateY(-2px);box-shadow:0 10px 20px rgba(102,126,234,.3)}
.btn-secondary{background:#fa0;color:#fff}
.btn-secondary:hover{background:#ff9500;transform:translateY(-2px);box-shadow:0 10px 20px rgba(255,170,0,.3)}
.btn:active{transform:translateY(0)}
.btn:disabled{opacity:.5;cursor:not-allowed}
.simulation-section{margin-top:20px}
.toggle-label{display:flex;align-items:center;gap:10px;cursor:pointer;font-weight:500;margin-bottom:15px}
.info-text{background:#fff3cd;padding:10px;border-radius:4px;font-size:.9em;color:#856404;border-left:3px solid #ffc107;margin-bottom:15px}
.code-block{background:#1e1e1e;color:#d4d4d4;padding:15px;border-radius:6px;font-family:'Courier New',monospace;font-size:.9em;line-height:1.5;overflow:auto;max-height:300px;border:1px solid #444}
.documentation{background:#f5f5f5;padding:30px;border-top:2px solid #ddd}
.documentation h3{color:#667eea;margin-bottom:20px;font-size:1.3em}
details{border:1px solid #ddd;border-radius:6px;padding:15px;background:#fff;margin-bottom:15px}
details summary{cursor:pointer;font-weight:600;color:#667eea;user-select:none;padding:10px;margin:-10px;border-radius:4px;transition:background .3s}
details summary:hover{background:#f0f4ff}
.doc-content{margin-top:15px;padding-top:15px;border-top:1px solid #eee}
.doc-content h4{color:#764ba2;margin:15px 0 10px}
.doc-content ul{margin:0 0 10px 20px}
.doc-content li{margin:5px 0;line-height:1.6}
.doc-content code{background:#f0f4ff;padding:2px 6px;border-radius:3px;font-family:'Courier New',monospace;color:#667eea}
.doc-content pre{background:#1e1e1e;color:#d4d4d4;padding:12px;border-radius:4px;font-family:'Courier New',monospace;font-size:.9em;overflow-x:auto;margin:10px 0}
.mapping-table{width:100%;border-collapse:collapse;margin:15px 0;background:#fff;border:1px solid #ddd}
.mapping-table th,.mapping-table td{padding:10px;text-align:left;border-bottom:1px solid #ddd}
.mapping-table th{background:#667eea;color:#fff;font-weight:600}
.mapping-table tr:nth-child(even){background:#f9f9f9}
.mapping-table tr:hover{background:#f0f4ff}
.btn:focus-visible,input:focus-visible,select:focus-visible{outline:2px solid #667eea;outline-offset:2px}
::-webkit-scrollbar{width:8px;height:8px}
::-webkit-scrollbar-track{background:#f1f1f1;border-radius:10px}
::-webkit-scrollbar-thumb{background:#667eea;border-radius:10px}
::-webkit-scrollbar-thumb:hover{background:#764ba2}
@media(max-width:1100px){.main-layout{grid-template-columns:1fr}}
@media(max-width:800px){.header h1{font-size:1.6em}.status-bar{gap:15px}.button-group,.controls-inline{flex-direction:column}.controls-inline{align-items:flex-start;width:100%}.section-header{flex-direction:column;align-items:flex-start}}
@media(max-width:600px){.config-grid{grid-template-columns:1fr}}
.init-mode-panel{background:#fff3cd;padding:15px;border-radius:6px;margin-top:15px;border:2px solid #ffc107}
.init-mode-panel .toggle-label span{font-size:1.05em}
.init-mode-panel.active{background:#d4edda;border-color:#28a745}
.init-status{font-size:.9em;font-weight:600;padding:6px 10px;border-radius:4px;margin-top:8px;text-align:center}
.init-status.locked{background:#28a745;color:#fff}
.init-status.free{background:#6c757d;color:#fff}
.console-section{background:#1a1a2e;padding:20px;border-top:3px solid #667eea}
.console-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:10px}
.console-header h3{color:#0f0;font-family:'Courier New',monospace;font-size:1.1em}
.console-controls{display:flex;gap:10px;align-items:center}
.console-controls button{background:#333;color:#ccc;border:1px solid #555;padding:5px 12px;border-radius:4px;cursor:pointer;font-size:.85em;transition:all .2s}
.console-controls button:hover{background:#555;color:#fff}
.console-controls button.active{background:#667eea;color:#fff;border-color:#667eea}
.console-status{font-size:.8em;color:#888;font-family:'Courier New',monospace}
#consoleOutput{background:#0d0d1a;color:#d4d4d4;font-family:'Courier New',monospace;font-size:.82em;line-height:1.6;padding:12px;border-radius:6px;height:250px;overflow-y:auto;border:1px solid #333;white-space:pre-wrap;word-break:break-all}
#consoleOutput .log-E{color:#f55}
#consoleOutput .log-W{color:#fa0}
#consoleOutput .log-I{color:#0c0}
#consoleOutput .log-D{color:#08f}
#consoleOutput .log-V{color:#888}
</style>
</head>
<body>
    <div class="container">
        <header class="header">
            <h1>🤖 Bittle Robot - Contrôle Inverse Cinématique</h1>
            <div class="status-bar">
                <div id="networkStatus" class="status-item">📡 État réseau...</div>
                <div class="status-item">Platform: ESP32-C6</div>
                <div class="status-item">LED: OFF</div>
            </div>
        </header>

        <div class="main-layout">
            <section class="ik-section">
                <div class="section-header">
                    <h2>📐 Visualisation IK</h2>
                    <div class="controls-inline">
                        <label for="legSelect">Patte:</label>
                        <select id="legSelect">
                            <option value="LF">LF (Avant-Gauche)</option>
                            <option value="LR">LR (Arrière-Gauche)</option>
                            <option value="RF">RF (Avant-Droit)</option>
                            <option value="RR">RR (Arrière-Droit)</option>
                        </select>
                        <label for="elbowUpToggle">
                            <input type="checkbox" id="elbowUpToggle"> Elbow-UP
                        </label>
                    </div>
                </div>
                <div id="canvasContainer" class="canvas-container">
                    <canvas id="ikCanvas" width="600" height="500"></canvas>
                </div>
                <div class="canvas-info">
                    <p>🖱️ Drag la cible (⭕ orange) pour modifier la position du pied.</p>
                    <p>Axe X (rouge) = horizontal | Axe Y (bleu) = vertical | Origine (0,0) = hanche</p>
                </div>
            </section>

            <section class="config-section">
                <h2>⚙️ Configuration Servo</h2>
                <div class="servo-config-panel">
                    <h3>HIP (Hanche)</h3>
                    <div class="config-grid">
                        <div class="config-input"><label for="hipOffsetInput">Offset (°):</label><input type="number" id="hipOffsetInput" value="0" step="1" min="-90" max="90"></div>
                        <div class="config-input"><label><input type="checkbox" id="hipInvertCheck"> Inverser</label></div>
                        <div class="config-input"><label for="hipMinInput">Min (°):</label><input type="number" id="hipMinInput" value="0" step="1" min="0" max="180"></div>
                        <div class="config-input"><label for="hipMaxInput">Max (°):</label><input type="number" id="hipMaxInput" value="180" step="1" min="0" max="180"></div>
                    </div>
                    <div class="servo-output">Angle Servo: <span id="hipOutputValue">90.0</span>° &nbsp; <label>Manuel: <input type="number" id="hipManualInput" value="90" step="1" min="0" max="180" style="width:60px;padding:4px;border:1px solid #667eea;border-radius:4px"></label></div>
                </div>
                <div class="servo-config-panel">
                    <h3>KNEE (Genou)</h3>
                    <div class="config-grid">
                        <div class="config-input"><label for="kneeOffsetInput">Offset (°):</label><input type="number" id="kneeOffsetInput" value="0" step="1" min="-90" max="90"></div>
                        <div class="config-input"><label><input type="checkbox" id="kneeInvertCheck"> Inverser</label></div>
                        <div class="config-input"><label for="kneeMinInput">Min (°):</label><input type="number" id="kneeMinInput" value="0" step="1" min="0" max="180"></div>
                        <div class="config-input"><label for="kneeMaxInput">Max (°):</label><input type="number" id="kneeMaxInput" value="180" step="1" min="0" max="180"></div>
                    </div>
                    <div class="servo-output">Angle Servo: <span id="kneeOutputValue">90.0</span>° &nbsp; <label>Manuel: <input type="number" id="kneeManualInput" value="90" step="1" min="0" max="180" style="width:60px;padding:4px;border:1px solid #667eea;border-radius:4px"></label></div>
                </div>
                <div class="button-group">
                    <button id="sendNeutralBtn" class="btn btn-secondary">🔄 Neutre (90°)</button>
                    <button id="standBtn" class="btn" style="background:#28a745;color:#fff">💪 Stand</button>
                    <button id="sendCurrentBtn" class="btn btn-primary">📤 Envoyer Angles</button>
                    <button id="resetConfigBtn" class="btn" style="background:#dc3545;color:#fff">🗑 Reset Config</button>
                </div>

                <div class="init-mode-panel">
                    <label for="initModeToggle" class="toggle-label">
                        <input type="checkbox" id="initModeToggle">
                        <span>🔒 Mode Initialisation</span>
                    </label>
                    <p class="info-text">Verrouille les servos à leur position actuelle. Décocher pour les libérer (servos libres, déplaçables à la main).</p>
                    <div id="initModeStatus" class="init-status">Servos libres</div>
                </div>
            </section>

            <section class="simulation-section">
                <h2>🎮 Mode Simulation</h2>
                <label for="simulationToggle" class="toggle-label">
                    <input type="checkbox" id="simulationToggle">
                    <span>Activer Mode Simulation</span>
                </label>
                <p class="info-text">En mode simulation, les commandes ne sont pas envoyées à l'ESP32, mais affichées en JSON pour debug.</p>
                <div id="simulationOutput" class="code-block">{ "leg": "LF", "angles": { "hipDeg": 0, "kneeDeg": 0 } }</div>
            </section>
        </div>

        <section class="console-section">
            <div class="console-header">
                <h3>&#x1F4DF; Serial Console</h3>
                <div class="console-controls">
                    <span id="consoleStatus" class="console-status">Connecting...</span>
                    <button id="consolePauseBtn" title="Pause/Resume">&#x23F8; Pause</button>
                    <button id="consoleClearBtn" title="Clear">&#x1F5D1; Clear</button>
                    <button id="consoleAutoScrollBtn" class="active" title="Auto-scroll">&#x2B07; Auto-scroll</button>
                </div>
            </div>
            <div id="consoleOutput"></div>
        </section>

        <footer class="documentation">
            <h3>📖 Documentation IK</h3>
            <details>
                <summary><strong>Conventions & Documentation</strong></summary>
                <div class="doc-content">
                    <h4>Repère Cartésien:</h4>
                    <ul>
                        <li><strong>Origine (0, 0)</strong> = articulation hanche</li>
                        <li><strong>Axe X</strong> = horizontal (positif vers l'avant)</li>
                        <li><strong>Axe Y</strong> = vertical (positif vers le bas)</li>
                        <li><strong>Unité</strong> = millimètres (mm)</li>
                    </ul>
                    <h4>Dimensions Physiques:</h4>
                    <ul>
                        <li><strong>L1 (Fémur)</strong> = 46 mm | <strong>L2 (Tibia)</strong> = 21 mm | <strong>Portée</strong> = 25–67 mm</li>
                    </ul>
                    <h4>Calcul IK:</h4>
                    <ul>
                        <li><code>r = √(x² + y²)</code> — <code>cosK = (r² - L1² - L2²) / (2·L1·L2)</code></li>
                        <li><code>K = acos(cosK)</code> — <code>A = atan2(y, x)</code> — <code>B = atan2(L2·sin(K), L1 + L2·cos(K))</code></li>
                        <li><strong>Elbow-DOWN:</strong> HIP = A - B | <strong>Elbow-UP:</strong> HIP = A + B</li>
                    </ul>
                    <h4>Mapping Servo PCA9685:</h4>
                    <table class="mapping-table">
                        <tr><th>Patte</th><th>Joint</th><th>Canal PCA</th><th>Servo Name</th></tr>
                        <tr><td rowspan="2">LF</td><td>HIP</td><td>0</td><td>LF_HIP</td></tr>
                        <tr><td>KNEE</td><td>1</td><td>LF_KNEE</td></tr>
                        <tr><td rowspan="2">LR</td><td>HIP</td><td>2</td><td>LR_HIP</td></tr>
                        <tr><td>KNEE</td><td>3</td><td>LR_KNEE</td></tr>
                        <tr><td rowspan="2">RF</td><td>HIP</td><td>4</td><td>RF_HIP</td></tr>
                        <tr><td>KNEE</td><td>5</td><td>RF_KNEE</td></tr>
                        <tr><td rowspan="2">RR</td><td>HIP</td><td>6</td><td>RR_HIP</td></tr>
                        <tr><td>KNEE</td><td>7</td><td>RR_KNEE</td></tr>
                    </table>
                    <h4>Configuration Servo:</h4>
                    <ul>
                        <li><strong>Offset</strong> = décalage ±90° | <strong>Invert</strong> = 180° - angle | <strong>Min/Max</strong> = limites 0-180°</li>
                    </ul>
                    <h4>Protocole d'envoi:</h4>
                    <ul>
                        <li><strong>HTTP REST:</strong> POST /api/servos | <strong>WebSocket:</strong> ws://192.168.4.1/ws</li>
                        <li><pre>{"leg":"LF","targets":{"x":40,"y":-30,"unit":"mm"},"angles":{"hipDeg":92.3,"kneeDeg":41.7},"servos":[{"name":"LF_HIP","pcaChannel":0,"deg":92.3},{"name":"LF_KNEE","pcaChannel":1,"deg":41.7}]}</pre></li>
                    </ul>
                </div>
            </details>
        </footer>
    </div>

<script>
(() => {
"use strict";

// ── Config ──
const L1 = 46, L2 = 21, MIN_R = 25, MAX_R = 67;
const LEGS = ["LF","LR","RF","RR"];

// Génération dynamique config servo (remplace 8 blocs répétitifs)
const SERVO_CFG_VERSION = 4;

const SERVO_CFG = {};
LEGS.forEach((leg, i) => {
    ["HIP","KNEE"].forEach((joint, j) => {
        const ch = i * 2 + j;
        const inv = (joint==="KNEE" && (leg==="RF"||leg==="RR")) || (joint==="HIP" && (leg==="LF"||leg==="LR"));
        const off = (joint==="KNEE") ? (inv ? -30 : 30) : 0;
        SERVO_CFG[`${leg}_${joint}`] = { name:`${leg}_${joint}`, pcaChannel:ch, offsetDeg:off, invert:inv, minDeg:0, maxDeg:180 };
    });
});

const getServoCfg = (leg, joint) => SERVO_CFG[`${leg}_${joint}`] || null;
const $ = id => document.getElementById(id);

function updateServoCfg(name, updates) {
    if (SERVO_CFG[name]) {
        Object.assign(SERVO_CFG[name], updates);
        localStorage.setItem("servoConfig", JSON.stringify({ version:SERVO_CFG_VERSION, data:SERVO_CFG }));
    }
}

function loadServoCfg() {
    try {
        const raw = JSON.parse(localStorage.getItem("servoConfig"));
        if (raw && raw.version === SERVO_CFG_VERSION && raw.data) {
            Object.keys(raw.data).forEach(k => { if (SERVO_CFG[k]) Object.assign(SERVO_CFG[k], raw.data[k]); });
        } else {
            localStorage.removeItem("servoConfig");
        }
    } catch(e) { localStorage.removeItem("servoConfig"); }
}

function resetAllConfig() {
    localStorage.removeItem("servoConfig");
    localStorage.removeItem("uiState");
    location.reload();
}

// ── IK Engine ──
function calculateIK(x, y, elbowUp = false) {
    let state = "ok", message = "", r = Math.hypot(x, y);
    if (r < MIN_R - 1) { state = "clamp"; message = `${r.toFixed(1)}mm < min ${MIN_R}mm`; r = MIN_R; }
    if (r > MAX_R + 1) { state = "clamp"; message = `${r.toFixed(1)}mm > max ${MAX_R}mm`; r = MAX_R; }

    let hipRad = 0, kneeRad = 0;
    try {
        const cosK = Math.max(-1, Math.min(1, (r*r - L1*L1 - L2*L2) / (2*L1*L2)));
        kneeRad = Math.acos(cosK);
        const A = Math.atan2(y, x);
        const B = Math.atan2(L2 * Math.sin(kneeRad), L1 + L2 * Math.cos(kneeRad));
        hipRad = elbowUp ? A + B : A - B;
        hipRad = ((hipRad + 3*Math.PI) % (2*Math.PI)) - Math.PI; // normalize [-π,π]
        if (elbowUp) kneeRad = -kneeRad;
    } catch(e) { state = "unreachable"; message = e.message; }

    const D = 180 / Math.PI;
    return { targetX:x, targetY:y, hipDeg:hipRad*D, kneeDeg:kneeRad*D, hipRad, kneeRad, distanceMM:r, state, message };
}

function applyServoCfg(ikDeg, cfg) {
    let d = cfg.invert ? 180 - ikDeg : ikDeg;
    return Math.max(cfg.minDeg, Math.min(cfg.maxDeg, d + cfg.offsetDeg));
}

function reverseServoCfg(servoDeg, cfg) {
    let d = servoDeg - cfg.offsetDeg;
    return cfg.invert ? 180 - d : d;
}

function forwardKinematics(hipDeg, kneeDeg) {
    const R = Math.PI / 180;
    const hR = hipDeg * R, kR = kneeDeg * R;
    const fx = L1 * Math.cos(hR) + L2 * Math.cos(hR + kR);
    const fy = L1 * Math.sin(hR) + L2 * Math.sin(hR + kR);
    return { x: fx, y: fy };
}

function clampTarget(x, y) {
    const r = Math.max(MIN_R, Math.min(MAX_R, Math.hypot(x, y))), a = Math.atan2(y, x);
    return { x: r * Math.cos(a), y: r * Math.sin(a) };
}

// ── UI ──
let ui = { leg:"LF", tx:40, ty:-30, elbowUp:false, ik:null, dragging:false, draggingKnee:false };
const legStates = {};
LEGS.forEach(leg => { legStates[leg] = { tx:40, ty:-30, elbowUp:false }; });
let canvas, ctx, ctnr;

function initUI() {
    canvas = $("ikCanvas"); ctx = canvas.getContext("2d"); ctnr = $("canvasContainer");

    const resize = () => { const r = ctnr.getBoundingClientRect(); canvas.width = r.width; canvas.height = r.height; draw(); };
    resize(); window.addEventListener("resize", resize);

    // Canvas mouse events
    canvas.addEventListener("mousedown", e => {
        const p = mouse(e);
        if (Math.hypot(p.x - ui.tx, p.y - ui.ty) < 15) { ui.dragging = true; return; }
        if (ui.ik) {
            const hR = ui.ik.hipDeg * Math.PI/180;
            const kneeX = L1 * Math.cos(hR), kneeY = L1 * Math.sin(hR);
            if (Math.hypot(p.x - kneeX, p.y - kneeY) < 15) { ui.draggingKnee = true; return; }
        }
    });
    canvas.addEventListener("mousemove", e => {
        if (!ui.dragging && !ui.draggingKnee) return;
        const p = mouse(e);
        if (ui.dragging) {
            const c = clampTarget(p.x, p.y);
            ui.tx = c.x; ui.ty = c.y;
        } else if (ui.draggingKnee && ui.ik) {
            const newHipRad = Math.atan2(p.y, p.x);
            const kneeDeg = ui.ik.kneeDeg;
            const kneeRad = kneeDeg * Math.PI / 180;
            const newFx = L1 * Math.cos(newHipRad) + L2 * Math.cos(newHipRad + kneeRad);
            const newFy = L1 * Math.sin(newHipRad) + L2 * Math.sin(newHipRad + kneeRad);
            const c = clampTarget(newFx, newFy);
            ui.tx = c.x; ui.ty = c.y;
        }
        saveUI(); draw(); updateServoValues();
    });
    const stop = () => { if (ui.dragging || ui.draggingKnee) { ui.dragging = false; ui.draggingKnee = false; draw(); } };
    canvas.addEventListener("mouseup", stop); canvas.addEventListener("mouseleave", stop);

    // Control events
    $("legSelect").addEventListener("change", e => {
        legStates[ui.leg] = { tx:ui.tx, ty:ui.ty, elbowUp:ui.elbowUp };
        ui.leg = e.target.value;
        const ls = legStates[ui.leg];
        ui.tx = ls.tx; ui.ty = ls.ty; ui.elbowUp = ls.elbowUp;
        $("elbowUpToggle").checked = ui.elbowUp;
        syncConfigPanel(); saveUI(); draw();
    });
    $("elbowUpToggle").addEventListener("change", e => { ui.elbowUp = e.target.checked; saveUI(); draw(); updateServoValues(); });
    $("sendNeutralBtn").addEventListener("click", sendNeutral);
    $("standBtn").addEventListener("click", sendStand);
    $("sendCurrentBtn").addEventListener("click", sendCurrent);
    $("resetConfigBtn").addEventListener("click", () => { if (confirm("Réinitialiser toute la configuration ?")) resetAllConfig(); });
    $("simulationToggle").addEventListener("change", e => { net.sim = e.target.checked; updateNetStatus(); });

    loadUI(); draw();
}

const getOrigin = () => ({ x: canvas.width/2, y: canvas.height/2 });
const getScale = () => Math.min(canvas.width, canvas.height) / 150;

function mouse(e) {
    const r = canvas.getBoundingClientRect(), o = getOrigin(), s = getScale();
    return { x: (e.clientX - r.left - o.x) / s, y: -(e.clientY - r.top - o.y) / s };
}

function draw() {
    const w = canvas.width, h = canvas.height, o = getOrigin(), s = getScale();
    ctx.fillStyle = "#fff"; ctx.fillRect(0, 0, w, h);

    // Grid 5mm
    const gs = 5 * s; ctx.strokeStyle = "#e0e0e0"; ctx.lineWidth = 0.5;
    for (let x = 0; x < w; x += gs) { ctx.beginPath(); ctx.moveTo(x,0); ctx.lineTo(x,h); ctx.stroke(); }
    for (let y = 0; y < h; y += gs) { ctx.beginPath(); ctx.moveTo(0,y); ctx.lineTo(w,y); ctx.stroke(); }

    // Axes
    const md = 100 * s;
    ctx.lineWidth = 2;
    // X (red)
    ctx.strokeStyle = "#f44"; ctx.beginPath(); ctx.moveTo(o.x-md,o.y); ctx.lineTo(o.x+md,o.y); ctx.stroke();
    ctx.fillStyle = "#f44"; ctx.beginPath(); ctx.moveTo(o.x+md,o.y); ctx.lineTo(o.x+md-10,o.y-5); ctx.lineTo(o.x+md-10,o.y+5); ctx.fill();
    // Y (blue)
    ctx.strokeStyle = "#44f"; ctx.beginPath(); ctx.moveTo(o.x,o.y+md); ctx.lineTo(o.x,o.y-md); ctx.stroke();
    ctx.fillStyle = "#44f"; ctx.beginPath(); ctx.moveTo(o.x,o.y-md); ctx.lineTo(o.x-5,o.y-md+10); ctx.lineTo(o.x+5,o.y-md+10); ctx.fill();
    // Origin + labels
    ctx.fillStyle = "#000"; ctx.beginPath(); ctx.arc(o.x,o.y,4,0,Math.PI*2); ctx.fill();
    ctx.fillStyle = "#666"; ctx.font = "12px Arial";
    ctx.fillText("+X", o.x+60*s+5, o.y-5); ctx.fillText("+Y", o.x+10, o.y-60*s-5);

    // IK + leg segments
    ui.ik = calculateIK(ui.tx, ui.ty, ui.elbowUp);
    const hR = ui.ik.hipDeg * Math.PI/180, kR = ui.ik.kneeDeg * Math.PI/180;
    const kx = o.x + L1*Math.cos(hR)*s, ky = o.y - L1*Math.sin(hR)*s;
    const fA = hR + kR, fx = kx + L2*Math.cos(fA)*s, fy = ky - L2*Math.sin(fA)*s;

    // Segments: L1 blue, L2 green
    [[o.x,o.y,kx,ky,"#06f"],[kx,ky,fx,fy,"#0c0"]].forEach(([x1,y1,x2,y2,c]) => {
        ctx.strokeStyle = c; ctx.lineWidth = 8; ctx.beginPath(); ctx.moveTo(x1,y1); ctx.lineTo(x2,y2); ctx.stroke();
    });
    // Joints
    [[o.x,o.y,"#f90"],[kx,ky,"#c0f"],[fx,fy,"#f06"]].forEach(([jx,jy,c]) => {
        ctx.fillStyle = c; ctx.beginPath(); ctx.arc(jx,jy,6,0,Math.PI*2); ctx.fill();
    });

    // Knee grab ring
    ctx.strokeStyle = ui.draggingKnee ? "#f00" : "#c0f"; ctx.lineWidth = 2;
    ctx.beginPath(); ctx.arc(kx,ky,10,0,Math.PI*2); ctx.stroke();

    // Target crosshair
    const tpx = o.x + ui.tx*s, tpy = o.y - ui.ty*s;
    ctx.strokeStyle = ui.dragging ? "#f00" : "#fa0"; ctx.lineWidth = 3;
    ctx.beginPath(); ctx.arc(tpx,tpy,10,0,Math.PI*2); ctx.stroke();
    ctx.strokeStyle = "#fa0"; ctx.lineWidth = 2;
    ctx.beginPath(); ctx.moveTo(tpx-5,tpy); ctx.lineTo(tpx+5,tpy); ctx.moveTo(tpx,tpy-5); ctx.lineTo(tpx,tpy+5); ctx.stroke();

    // Info text overlay
    const ik = ui.ik, lh = 18; ctx.textBaseline = "top"; ctx.font = "14px monospace"; let iy = 10;
    const stateColor = {ok:"#0c0",clamp:"#fa0",unreachable:"#f33"}[ik.state] || "#999";
    [
        ["#333", `📍 Patte: ${ui.leg}`],
        ["#333", `🎯 Cible: (${ui.tx.toFixed(1)}, ${ui.ty.toFixed(1)}) mm`],
        ["#06f", `📏 Distance: ${ik.distanceMM.toFixed(1)} mm`],
        ["#0a0", `📐 HIP:  ${ik.hipDeg.toFixed(1)}° (${ik.hipRad.toFixed(3)} rad)`],
        ["#0a0", `📐 KNEE: ${ik.kneeDeg.toFixed(1)}° (${ik.kneeRad.toFixed(3)} rad)`],
        [stateColor, `⚠️ État: ${ik.state}${ik.message ? " — "+ik.message : ""}`],
        ["#333", `🦵 Mode: ${ui.elbowUp ? "Elbow-UP" : "Elbow-DOWN"}`],
    ].forEach(([c,t]) => { ctx.fillStyle = c; ctx.fillText(t, 10, iy); iy += lh; });

    updateServoValues();
}

// ── Servo Config Panel ──
function syncConfigPanel() {
    const h = getServoCfg(ui.leg, "HIP"), k = getServoCfg(ui.leg, "KNEE");
    if (!h || !k) return;
    $("hipOffsetInput").value = h.offsetDeg; $("hipInvertCheck").checked = h.invert;
    $("hipMinInput").value = h.minDeg; $("hipMaxInput").value = h.maxDeg;
    $("kneeOffsetInput").value = k.offsetDeg; $("kneeInvertCheck").checked = k.invert;
    $("kneeMinInput").value = k.minDeg; $("kneeMaxInput").value = k.maxDeg;
    updateServoValues();
}

function updateServoValues() {
    if (!ui.ik) return;
    const h = getServoCfg(ui.leg,"HIP"), k = getServoCfg(ui.leg,"KNEE");
    if (h) { const v = applyServoCfg(ui.ik.hipDeg, h).toFixed(1); $("hipOutputValue").textContent = v; $("hipManualInput").value = Math.round(+v); }
    if (k) { const v = applyServoCfg(ui.ik.kneeDeg, k).toFixed(1); $("kneeOutputValue").textContent = v; $("kneeManualInput").value = Math.round(+v); }
}

function applyManualServoAngles() {
    const h = getServoCfg(ui.leg,"HIP"), k = getServoCfg(ui.leg,"KNEE");
    if (!h || !k) return;
    const hipServo = +$("hipManualInput").value || 0;
    const kneeServo = +$("kneeManualInput").value || 0;
    const hipIK = reverseServoCfg(hipServo, h);
    const kneeIK = reverseServoCfg(kneeServo, k);
    const pos = forwardKinematics(hipIK, kneeIK);
    const c = clampTarget(pos.x, pos.y);
    ui.tx = c.x; ui.ty = c.y;
    saveUI(); draw();
}

function setupServoListeners() {
    ["hipOffsetInput","hipInvertCheck","hipMinInput","hipMaxInput",
     "kneeOffsetInput","kneeInvertCheck","kneeMinInput","kneeMaxInput"].forEach(id =>
        $(id).addEventListener("change", () => {
            updateServoCfg(`${ui.leg}_HIP`, {
                offsetDeg:+$("hipOffsetInput").value||0, invert:$("hipInvertCheck").checked,
                minDeg:+$("hipMinInput").value||0, maxDeg:+$("hipMaxInput").value||180
            });
            updateServoCfg(`${ui.leg}_KNEE`, {
                offsetDeg:+$("kneeOffsetInput").value||0, invert:$("kneeInvertCheck").checked,
                minDeg:+$("kneeMinInput").value||0, maxDeg:+$("kneeMaxInput").value||180
            });
            syncConfigPanel();
        })
    );
    $("hipManualInput").addEventListener("change", applyManualServoAngles);
    $("kneeManualInput").addEventListener("change", applyManualServoAngles);
}

// ── Persistence ──
function saveUI() {
    legStates[ui.leg] = { tx:ui.tx, ty:ui.ty, elbowUp:ui.elbowUp };
    localStorage.setItem("uiState", JSON.stringify({ leg:ui.leg, legStates }));
}
function loadUI() {
    try {
        const s = JSON.parse(localStorage.getItem("uiState"));
        if (s) {
            ui.leg = s.leg || ui.leg;
            if (s.legStates) {
                LEGS.forEach(leg => { if (s.legStates[leg]) Object.assign(legStates[leg], s.legStates[leg]); });
            } else {
                // backward compat: ancien format sans legStates
                legStates[ui.leg] = { tx:s.tx??40, ty:s.ty??-30, elbowUp:!!s.elbowUp };
            }
            const ls = legStates[ui.leg];
            ui.tx = ls.tx; ui.ty = ls.ty; ui.elbowUp = ls.elbowUp;
        }
        $("legSelect").value = ui.leg; $("elbowUpToggle").checked = ui.elbowUp;
        syncConfigPanel();
    } catch(e) {}
}

// ── Network ──
let net = { connected:false, wsOk:false, ip:"192.168.4.1", port:80, proto:"http", sim:false, lastSent:0, throttle:50 };
let ws = null;

function initNet() {
    if (net.proto === "ws") connectWS(); else net.connected = true;
    updateNetStatus();
}

function connectWS() {
    try {
        ws = new WebSocket(`ws://${net.ip}:${net.port}/ws`);
        ws.onopen = () => { net.wsOk = net.connected = true; updateNetStatus(); };
        ws.onclose = () => { net.wsOk = net.connected = false; updateNetStatus(); setTimeout(connectWS, 5000); };
        ws.onerror = () => { net.wsOk = false; updateNetStatus(); };
    } catch(e) { net.wsOk = false; }
}

function updateNetStatus() {
    const el = $("networkStatus"); if (!el) return;
    el.innerHTML = net.sim ? "🎮 <strong>SIMULATION</strong> - Pas de réseau"
        : net.connected ? `✅ <strong>Connecté</strong> (${net.proto==="ws"?"WebSocket":"HTTP"}) - ${net.ip}`
        : `❌ <strong>Déconnecté</strong> - ${net.ip}`;
}

function buildPayload(leg, angles) {
    const h = getServoCfg(leg,"HIP"), k = getServoCfg(leg,"KNEE");
    if (!h || !k) throw new Error(`Invalid leg: ${leg}`);
    return {
        leg, targets:{x:angles.targetX||0,y:angles.targetY||0,unit:"mm"},
        angles:{hipDeg:angles.hipDeg,kneeDeg:angles.kneeDeg},
        servos:[
            {name:h.name, pcaChannel:h.pcaChannel, deg:applyServoCfg(angles.hipDeg,h)},
            {name:k.name, pcaChannel:k.pcaChannel, deg:applyServoCfg(angles.kneeDeg,k)}
        ]
    };
}

async function sendPayload(payload) {
    const now = Date.now();
    if (now - net.lastSent < net.throttle) return false;
    net.lastSent = now;
    if (net.sim) { $("simulationOutput").textContent = JSON.stringify(payload,null,2); return true; }
    if (!net.connected) return false;
    try {
        if (net.proto==="ws" && net.wsOk) ws.send(JSON.stringify(payload));
        else {
            const r = await fetch(`http://${net.ip}:${net.port}/api/servos`,
                {method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify(payload)});
            if (!r.ok) throw new Error(`HTTP ${r.status}`);
            const json = await r.json();
            console.log("Server response:", json);
            if (json.applied === 0) console.warn("WARN: server applied 0 servos!", json);
        }
        return true;
    } catch(e) { console.error("Send failed:",e); return false; }
}

function sendStand() {
    const STAND_HIP_IK = 90, STAND_KNEE_IK = -10;
    const servos = [];
    LEGS.forEach(leg => {
        const hCfg = getServoCfg(leg,"HIP"), kCfg = getServoCfg(leg,"KNEE");
        if (!hCfg || !kCfg) return;
        const pos = forwardKinematics(STAND_HIP_IK, STAND_KNEE_IK);
        legStates[leg] = { tx:pos.x, ty:pos.y, elbowUp:false };
        servos.push({name:hCfg.name, pcaChannel:hCfg.pcaChannel, deg:applyServoCfg(STAND_HIP_IK, hCfg)});
        servos.push({name:kCfg.name, pcaChannel:kCfg.pcaChannel, deg:applyServoCfg(STAND_KNEE_IK, kCfg)});
    });
    const ls = legStates[ui.leg];
    ui.tx = ls.tx; ui.ty = ls.ty; ui.elbowUp = false;
    $("elbowUpToggle").checked = false;
    saveUI(); draw(); updateServoValues();
    const payload = {command:"stand",servos};
    if (net.sim) { $("simulationOutput").textContent = JSON.stringify(payload,null,2); return; }
    sendPayload(payload);
}

function sendNeutral() {
    const servos = [];
    LEGS.forEach(leg => {
        const hCfg = getServoCfg(leg,"HIP"), kCfg = getServoCfg(leg,"KNEE");
        if (!hCfg || !kCfg) return;
        const hipIK = reverseServoCfg(90, hCfg), kneeIK = reverseServoCfg(90, kCfg);
        const pos = forwardKinematics(hipIK, kneeIK);
        legStates[leg] = { tx:pos.x, ty:pos.y, elbowUp:false };
        ["HIP","KNEE"].forEach(j => {
            const c = getServoCfg(leg,j); if (c) servos.push({name:c.name,pcaChannel:c.pcaChannel,deg:90});
        });
    });
    const ls = legStates[ui.leg];
    ui.tx = ls.tx; ui.ty = ls.ty; ui.elbowUp = false;
    $("elbowUpToggle").checked = false;
    saveUI(); draw(); updateServoValues();
    const payload = {command:"neutral",servos};
    if (net.sim) { $("simulationOutput").textContent = JSON.stringify(payload,null,2); return; }
    sendPayload(payload);
}

function sendCurrent() {
    if (!ui.ik) return;
    sendPayload(buildPayload(ui.leg, {hipDeg:ui.ik.hipDeg,kneeDeg:ui.ik.kneeDeg,targetX:ui.tx,targetY:ui.ty}));
}

// ── Init ──
document.addEventListener("DOMContentLoaded", () => {
    loadServoCfg(); initUI(); initNet(); setupServoListeners(); initConsole(); initInitMode();
});

window.sendPayload = sendPayload;

// ── Init Mode (lock/release servos) ──
function initInitMode() {
    const toggle = $("initModeToggle");
    toggle.addEventListener("change", async () => {
        const enabled = toggle.checked;
        const panel = toggle.closest(".init-mode-panel");
        const status = $("initModeStatus");
        try {
            const r = await fetch("/api/init-mode", {
                method:"POST",
                headers:{"Content-Type":"application/json"},
                body: JSON.stringify({enabled})
            });
            if (!r.ok) throw new Error(`HTTP ${r.status}`);
            const data = await r.json();
            if (enabled) {
                panel.classList.add("active");
                status.textContent = "🔒 Servos verrouillés (" + (data.angles||[]).join(", ") + ")°";
                status.className = "init-status locked";
            } else {
                panel.classList.remove("active");
                status.textContent = "Servos libres";
                status.className = "init-status free";
            }
        } catch(e) {
            console.error("Init mode error:", e);
            status.textContent = "❌ Erreur: " + e.message;
            status.className = "init-status";
            toggle.checked = !enabled;
        }
    });
}

// ── Serial Console ──
let conState = { since: 0, paused: false, autoScroll: true, timer: null, lineCount: 0 };

function initConsole() {
    const out = $("consoleOutput");
    $("consolePauseBtn").addEventListener("click", () => {
        conState.paused = !conState.paused;
        $("consolePauseBtn").innerHTML = conState.paused ? "&#x25B6; Resume" : "&#x23F8; Pause";
        $("consolePauseBtn").classList.toggle("active", conState.paused);
    });
    $("consoleClearBtn").addEventListener("click", () => {
        out.innerHTML = "";
        conState.lineCount = 0;
    });
    $("consoleAutoScrollBtn").addEventListener("click", () => {
        conState.autoScroll = !conState.autoScroll;
        $("consoleAutoScrollBtn").classList.toggle("active", conState.autoScroll);
    });

    // Start polling
    conState.timer = setInterval(pollLogs, 1000);
    pollLogs();
}

function classifyLogLine(line) {
    if (/^E \(|^E\s|ESP_ERROR|FAIL|ERROR/i.test(line)) return "log-E";
    if (/^W \(|^W\s|WARN/i.test(line)) return "log-W";
    if (/^I \(|^I\s/i.test(line)) return "log-I";
    if (/^D \(|^D\s/i.test(line)) return "log-D";
    if (/^V \(|^V\s/i.test(line)) return "log-V";
    return "";
}

async function pollLogs() {
    if (conState.paused) return;
    const status = $("consoleStatus");
    try {
        const r = await fetch(`/api/logs?since=${conState.since}`);
        if (!r.ok) throw new Error(`HTTP ${r.status}`);
        const data = await r.json();
        status.textContent = `Connected | ${data.idx} lines total`;
        status.style.color = "#0c0";

        if (data.lines && data.lines.length > 0) {
            const out = $("consoleOutput");
            const frag = document.createDocumentFragment();
            data.lines.forEach(line => {
                const div = document.createElement("div");
                const cls = classifyLogLine(line);
                if (cls) div.className = cls;
                div.textContent = line;
                frag.appendChild(div);
                conState.lineCount++;
            });
            out.appendChild(frag);

            // Limit DOM lines to 500
            while (out.children.length > 500) out.removeChild(out.firstChild);

            if (conState.autoScroll) out.scrollTop = out.scrollHeight;
            conState.since = data.idx;
        }
    } catch(e) {
        status.textContent = "Disconnected";
        status.style.color = "#f55";
    }
}

})();
</script>
</body>
</html>
)";
