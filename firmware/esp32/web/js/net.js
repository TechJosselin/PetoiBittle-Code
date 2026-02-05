/**
 * net.js - Communication réseau avec l'ESP32
 * Support HTTP REST et WebSocket
 */

import { applyServoConfig, getServoConfig } from "./config.js";
import { SERVO_CONFIG, SERVO_MAPPING } from "./config.js";

// État réseau
let networkState = {
  isConnected: false,
  wsConnected: false,
  ip: "192.168.4.1",
  port: 80,
  protocol: "http", // "http" ou "ws"
  simulationMode: false,
  lastSentTime: 0,
  sendThrottleMs: 50,
};

let ws = null;

/**
 * Initialise le module réseau
 * @param {string} ip - Adresse IP de l'ESP32
 * @param {string} protocol - "http" ou "ws"
 */
export function initNetwork(ip = "192.168.4.1", protocol = "http") {
  networkState.ip = ip;
  networkState.protocol = protocol;

  if (protocol === "ws") {
    connectWebSocket();
  } else {
    networkState.isConnected = true;
  }

  // Listener pour événements de l'UI
  document.addEventListener("sendNeutral", handleSendNeutral);
  document.addEventListener("sendCurrent", handleSendCurrent);
  document.addEventListener("targetChanged", handleTargetChanged);
  document.addEventListener("simulationToggled", handleSimulationToggled);

  console.log(`✓ Network module initialized (${protocol} mode)`);
}

/**
 * Se connecte au WebSocket
 */
function connectWebSocket() {
  const wsUrl = `ws://${networkState.ip}:${networkState.port}/ws`;
  console.log(`Connecting to WebSocket: ${wsUrl}`);

  try {
    ws = new WebSocket(wsUrl);

    ws.addEventListener("open", () => {
      networkState.wsConnected = true;
      networkState.isConnected = true;
      console.log("✓ WebSocket connected");
      updateNetworkStatus();
    });

    ws.addEventListener("close", () => {
      networkState.wsConnected = false;
      networkState.isConnected = false;
      console.log("✗ WebSocket disconnected");
      updateNetworkStatus();
      // Retry après 5 secondes
      setTimeout(connectWebSocket, 5000);
    });

    ws.addEventListener("error", (err) => {
      console.error("✗ WebSocket error:", err);
      networkState.wsConnected = false;
      updateNetworkStatus();
    });
  } catch (e) {
    console.error("✗ Failed to connect WebSocket:", e);
    networkState.wsConnected = false;
  }
}

/**
 * Envoie les angles des servos
 * @param {object} servoData - {leg, angles: {hipDeg, kneeDeg}}
 */
export async function sendServos(servoData) {
  if (networkState.simulationMode) {
    console.log("[SIMULATION] Servo command:", servoData);
    displaySimulationOutput(servoData);
    return true;
  }

  // Throttle les envois
  const now = Date.now();
  if (now - networkState.lastSentTime < networkState.sendThrottleMs) {
    return false;
  }
  networkState.lastSentTime = now;

  if (!networkState.isConnected) {
    console.warn("⚠️ Not connected to ESP32");
    return false;
  }

  try {
    const payload = buildServoPayload(servoData);
    console.log("📤 Sending:", payload);

    if (networkState.protocol === "ws" && networkState.wsConnected) {
      ws.send(JSON.stringify(payload));
    } else {
      // HTTP REST
      const response = await fetch(`http://${networkState.ip}:${networkState.port}/api/servos`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload),
      });

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}`);
      }
    }

    return true;
  } catch (e) {
    console.error("✗ Failed to send servos:", e);
    return false;
  }
}

/**
 * Construit le payload normalisé pour envoi
 * @param {object} servoData - {leg, angles: {hipDeg, kneeDeg}}
 * @returns {object} Payload JSON
 */
function buildServoPayload(servoData) {
  const { leg, angles } = servoData;

  // Récupérer les configurations servo
  const hipConfig = getServoConfig(leg, "HIP");
  const kneeConfig = getServoConfig(leg, "KNEE");

  if (!hipConfig || !kneeConfig) {
    throw new Error(`Invalid leg: ${leg}`);
  }

  // Appliquer offsets et inversions
  const hipServoDeg = applyServoConfig(angles.hipDeg, hipConfig);
  const kneeServoDeg = applyServoConfig(angles.kneeDeg, kneeConfig);

  const payload = {
    leg,
    targets: {
      x: angles.targetX || 0,
      y: angles.targetY || 0,
      unit: "mm",
    },
    angles: {
      hipDeg: angles.hipDeg,
      kneeDeg: angles.kneeDeg,
    },
    servos: [
      {
        name: hipConfig.name,
        pcaChannel: hipConfig.pcaChannel,
        deg: hipServoDeg,
      },
      {
        name: kneeConfig.name,
        pcaChannel: kneeConfig.pcaChannel,
        deg: kneeServoDeg,
      },
    ],
  };

  return payload;
}

/**
 * Envoie une position neutre (90°) pour tous les servos d'une patte
 */
function handleSendNeutral(e) {
  const { leg } = e.detail;
  console.log(`🔄 Sending neutral (90°) to ${leg}`);

  sendServos({
    leg,
    angles: {
      hipDeg: 90,
      kneeDeg: 90,
      targetX: 0,
      targetY: 0,
    },
  });
}

/**
 * Envoie les angles actuels calculés
 */
function handleSendCurrent(e) {
  const { leg } = e.detail;
  const uiState = getUIStateFromDOM();

  if (uiState.leg !== leg) {
    console.warn(`⚠️ Leg mismatch: expected ${leg}, got ${uiState.leg}`);
    return;
  }

  console.log(`📤 Sending current angles for ${leg}`);

  sendServos({
    leg,
    angles: {
      hipDeg: uiState.hipDeg,
      kneeDeg: uiState.kneeDeg,
      targetX: uiState.targetX,
      targetY: uiState.targetY,
    },
  });
}

/**
 * Envoie les angles lors d'un changement de cible (throttled)
 */
function handleTargetChanged(e) {
  // Implémentation optionnelle: auto-send au changement de cible
  // À décommenter si souhaité
  /*
  const { x, y } = e.detail;
  console.log(`🎯 Target changed: (${x}, ${y})`);
  */
}

/**
 * Active/désactive le mode simulation
 */
function handleSimulationToggled(e) {
  const { simulation } = e.detail;
  networkState.simulationMode = simulation;
  console.log(`Mode simulation: ${simulation ? "ON" : "OFF"}`);
  updateNetworkStatus();
}

/**
 * Met à jour l'affichage de l'état réseau
 */
function updateNetworkStatus() {
  const statusEl = document.getElementById("networkStatus");
  if (!statusEl) return;

  if (networkState.simulationMode) {
    statusEl.innerHTML = "🎮 <strong>SIMULATION MODE</strong> - Pas de réseau";
  } else if (networkState.isConnected) {
    const proto = networkState.protocol === "ws" ? "WebSocket" : "HTTP REST";
    statusEl.innerHTML = `✅ <strong>Connecté</strong> (${proto}) - ${networkState.ip}`;
  } else {
    statusEl.innerHTML = `❌ <strong>Déconnecté</strong> - ${networkState.ip}`;
  }
}

/**
 * Affiche le JSON de commande en mode simulation
 */
function displaySimulationOutput(servoData) {
  const outputEl = document.getElementById("simulationOutput");
  if (!outputEl) return;

  const payload = buildServoPayload(servoData);
  outputEl.textContent = JSON.stringify(payload, null, 2);
}

/**
 * Récupère l'état actuel de l'UI depuis le DOM
 */
function getUIStateFromDOM() {
  const legSelect = document.getElementById("legSelect");
  const hipOutput = document.getElementById("hipOutputValue");
  const kneeOutput = document.getElementById("kneeOutputValue");

  // Récupérer aussi la cible depuis le canvas
  const canvasEl = document.getElementById("ikCanvas");
  const targetX = parseFloat(canvasEl.dataset.targetX) || 0;
  const targetY = parseFloat(canvasEl.dataset.targetY) || 0;

  return {
    leg: legSelect?.value || "LF",
    hipDeg: parseFloat(hipOutput?.textContent) || 90,
    kneeDeg: parseFloat(kneeOutput?.textContent) || 90,
    targetX,
    targetY,
  };
}

/**
 * Exporte les fonctions publiques
 */
export function getNetworkState() {
  return { ...networkState };
}

/**
 * Teste la connexion à l'ESP32
 */
export async function testConnection() {
  try {
    const response = await fetch(`http://${networkState.ip}:${networkState.port}/status`);
    const data = await response.json();
    console.log("✓ ESP32 Status:", data);
    return data;
  } catch (e) {
    console.error("✗ Failed to connect to ESP32:", e);
    return null;
  }
}

/**
 * Configure le mode de debug
 */
export function setDebugMode(enabled) {
  if (enabled) {
    // Log tous les envois réseau
    console.log("🐛 Debug mode enabled");
  }
}
