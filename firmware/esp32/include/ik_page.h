#pragma once

/**
 * ik_page.h - Page HTML IK générée automatiquement depuis web/index.html
 * ATTENTION: Ce fichier est généré automatiquement par pre_build_hook.py
 * Ne pas éditer directement - modifiez web/index.html à la place !
 */

static const char* HTML_IK_PAGE = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🤖 Bittle Robot IK Control</title>
    <link rel="stylesheet" href="styles.css">
</head>
<body>
    <div class="container">
        <!-- En-tête -->
        <header class="header">
            <h1>🤖 Bittle Robot - Contrôle Inverse Cinématique</h1>
            <div class="status-bar">
                <div id="networkStatus" class="status-item">📡 État réseau...</div>
                <div id="platformStatus" class="status-item">Platform: ESP32-C6</div>
                <div id="ledStatus" class="status-item">LED: OFF</div>
            </div>
        </header>

        <!-- Contenu principal -->
        <div class="main-layout">
            <!-- Section IK Canvas -->
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
                            <input type="checkbox" id="elbowUpToggle">
                            Elbow-UP
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

            <!-- Section Configuration Servos -->
            <section class="config-section">
                <h2>⚙️ Configuration Servo</h2>

                <!-- HIP (Hanche) -->
                <div class="servo-config-panel">
                    <h3>HIP (Hanche)</h3>
                    <div class="config-grid">
                        <div class="config-input">
                            <label for="hipOffsetInput">Offset (°):</label>
                            <input type="number" id="hipOffsetInput" value="0" step="1" min="-90" max="90">
                        </div>
                        <div class="config-input">
                            <label>
                                <input type="checkbox" id="hipInvertCheck">
                                Inverser
                            </label>
                        </div>
                        <div class="config-input">
                            <label for="hipMinInput">Min (°):</label>
                            <input type="number" id="hipMinInput" value="0" step="1" min="0" max="180">
                        </div>
                        <div class="config-input">
                            <label for="hipMaxInput">Max (°):</label>
                            <input type="number" id="hipMaxInput" value="180" step="1" min="0" max="180">
                        </div>
                    </div>
                    <div class="servo-output">
                        Angle Servo: <span id="hipOutputValue">90.0</span>°
                    </div>
                </div>

                <!-- KNEE (Genou) -->
                <div class="servo-config-panel">
                    <h3>KNEE (Genou)</h3>
                    <div class="config-grid">
                        <div class="config-input">
                            <label for="kneeOffsetInput">Offset (°):</label>
                            <input type="number" id="kneeOffsetInput" value="0" step="1" min="-90" max="90">
                        </div>
                        <div class="config-input">
                            <label>
                                <input type="checkbox" id="kneeInvertCheck">
                                Inverser
                            </label>
                        </div>
                        <div class="config-input">
                            <label for="kneeMinInput">Min (°):</label>
                            <input type="number" id="kneeMinInput" value="0" step="1" min="0" max="180">
                        </div>
                        <div class="config-input">
                            <label for="kneeMaxInput">Max (°):</label>
                            <input type="number" id="kneeMaxInput" value="180" step="1" min="0" max="180">
                        </div>
                    </div>
                    <div class="servo-output">
                        Angle Servo: <span id="kneeOutputValue">90.0</span>°
                    </div>
                </div>

                <!-- Boutons de contrôle -->
                <div class="button-group">
                    <button id="sendNeutralBtn" class="btn btn-secondary">
                        🔄 Neutre (90°)
                    </button>
                    <button id="sendCurrentBtn" class="btn btn-primary">
                        📤 Envoyer Angles
                    </button>
                </div>
            </section>

            <!-- Section Mode Simulation -->
            <section class="simulation-section">
                <h2>🎮 Mode Simulation</h2>
                <label for="simulationToggle" class="toggle-label">
                    <input type="checkbox" id="simulationToggle">
                    <span>Activer Mode Simulation</span>
                </label>
                <p class="info-text">En mode simulation, les commandes ne sont pas envoyées à l'ESP32, 
                   mais affichées en JSON pour debug.</p>
                <div id="simulationOutput" class="code-block">
{
  "leg": "LF",
  "angles": {
    "hipDeg": 0,
    "kneeDeg": 0
  }
}
                </div>
            </section>
        </div>

        <!-- Pied de page avec documentation -->
        <footer class="documentation">
            <h3>📖 Documentation IK</h3>
            <details>
                <summary><strong>Conventions & Conventions</strong></summary>
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
                        <li><strong>L1 (Fémur)</strong> = 46 mm (hanche → genou)</li>
                        <li><strong>L2 (Tibia)</strong> = 21 mm (genou → pied)</li>
                        <li><strong>Portée</strong> = 25 mm à 67 mm</li>
                    </ul>

                    <h4>Calcul IK:</h4>
                    <ul>
                        <li><code>r = √(x² + y²)</code> = distance hanche-pied</li>
                        <li><code>cosK = (r² - L1² - L2²) / (2·L1·L2)</code></li>
                        <li><code>K = acos(cosK)</code> = angle interne genou (radians)</li>
                        <li><code>A = atan2(y, x)</code> = angle approche</li>
                        <li><code>B = atan2(L2·sin(K), L1 + L2·cos(K))</code></li>
                        <li><strong>Elbow-DOWN (défaut):</strong> HIP = A - B</li>
                        <li><strong>Elbow-UP:</strong> HIP = A + B</li>
                    </ul>

                    <h4>Mapping Servo PCA9685:</h4>
                    <table class="mapping-table">
                        <tr>
                            <th>Patte</th>
                            <th>Joint</th>
                            <th>Canal PCA</th>
                            <th>Servo Name</th>
                        </tr>
                        <tr>
                            <td rowspan="2">LF (Avant-Gauche)</td>
                            <td>HIP</td>
                            <td>0</td>
                            <td>LF_HIP</td>
                        </tr>
                        <tr>
                            <td>KNEE</td>
                            <td>1</td>
                            <td>LF_KNEE</td>
                        </tr>
                        <tr>
                            <td rowspan="2">LR (Arrière-Gauche)</td>
                            <td>HIP</td>
                            <td>2</td>
                            <td>LR_HIP</td>
                        </tr>
                        <tr>
                            <td>KNEE</td>
                            <td>3</td>
                            <td>LR_KNEE</td>
                        </tr>
                        <tr>
                            <td rowspan="2">RF (Avant-Droit)</td>
                            <td>HIP</td>
                            <td>4</td>
                            <td>RF_HIP</td>
                        </tr>
                        <tr>
                            <td>KNEE</td>
                            <td>5</td>
                            <td>RF_KNEE</td>
                        </tr>
                        <tr>
                            <td rowspan="2">RR (Arrière-Droit)</td>
                            <td>HIP</td>
                            <td>6</td>
                            <td>RR_HIP</td>
                        </tr>
                        <tr>
                            <td>KNEE</td>
                            <td>7</td>
                            <td>RR_KNEE</td>
                        </tr>
                    </table>

                    <h4>Configuration Servo:</h4>
                    <ul>
                        <li><strong>Offset</strong> = décalage appliqué à l'angle IK (±90°)</li>
                        <li><strong>Invert</strong> = inverse l'angle (180° - angle)</li>
                        <li><strong>Min/Max</strong> = limites de mouvement du servo (0-180°)</li>
                    </ul>

                    <h4>Protocole d'envoi:</h4>
                    <ul>
                        <li><strong>HTTP REST:</strong> POST /api/servos avec body JSON</li>
                        <li><strong>WebSocket:</strong> ws://192.168.4.1/ws</li>
                        <li><strong>Format:</strong> 
                            <pre>{
  "leg": "LF",
  "targets": {"x": 40, "y": -30, "unit": "mm"},
  "angles": {"hipDeg": 92.3, "kneeDeg": 41.7},
  "servos": [
    {"name": "LF_HIP", "pcaChannel": 0, "deg": 92.3},
    {"name": "LF_KNEE", "pcaChannel": 1, "deg": 41.7}
  ]
}</pre>
                        </li>
                    </ul>
                </div>
            </details>
        </footer>
    </div>

    <!-- Scripts -->
    <script type="module">
        import { initUI, updateServoConfigPanel } from "./js/ui.js";
        import { initNetwork, sendServos } from "./js/net.js";
        import { loadServoConfig, updateServoConfig } from "./js/config.js";

        // Initialisation
        document.addEventListener("DOMContentLoaded", () => {
            console.log("🚀 Initialisation...");

            // Charger la config servo depuis localStorage
            loadServoConfig();

            // Initialiser l'UI
            initUI();

            // Initialiser le réseau (mode HTTP par défaut)
            initNetwork("192.168.4.1", "http");

            // Event listeners pour les changements de config servo
            setupServoConfigListeners();

            console.log("✓ Application ready");
        });

        /**
         * Configure les listeners pour les modifications de configuration servo
         */
        function setupServoConfigListeners() {
            const inputs = document.querySelectorAll(
                "#hipOffsetInput, #hipInvertCheck, #hipMinInput, #hipMaxInput, " +
                "#kneeOffsetInput, #kneeInvertCheck, #kneeMinInput, #kneeMaxInput"
            );

            inputs.forEach((input) => {
                input.addEventListener("change", () => {
                    const legSelect = document.getElementById("legSelect");
                    const leg = legSelect?.value || "LF";

                    // Récupérer les valeurs actuelles
                    const hipOffset = parseFloat(document.getElementById("hipOffsetInput").value) || 0;
                    const hipInvert = document.getElementById("hipInvertCheck").checked;
                    const hipMin = parseFloat(document.getElementById("hipMinInput").value) || 0;
                    const hipMax = parseFloat(document.getElementById("hipMaxInput").value) || 180;

                    const kneeOffset = parseFloat(document.getElementById("kneeOffsetInput").value) || 0;
                    const kneeInvert = document.getElementById("kneeInvertCheck").checked;
                    const kneeMin = parseFloat(document.getElementById("kneeMinInput").value) || 0;
                    const kneeMax = parseFloat(document.getElementById("kneeMaxInput").value) || 180;

                    // Mettre à jour la configuration
                    updateServoConfig(`${leg}_HIP`, {
                        offsetDeg: hipOffset,
                        invert: hipInvert,
                        minDeg: hipMin,
                        maxDeg: hipMax,
                    });

                    updateServoConfig(`${leg}_KNEE`, {
                        offsetDeg: kneeOffset,
                        invert: kneeInvert,
                        minDeg: kneeMin,
                        maxDeg: kneeMax,
                    });

                    // Redessiner le canvas
                    updateServoConfigPanel();
                });
            });
        }

        // Exposer globalement pour debug
        window.sendServos = sendServos;
    </script>
</body>
</html>

)rawliteral";
