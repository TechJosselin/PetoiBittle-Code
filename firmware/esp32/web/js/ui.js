/**
 * ui.js - Interface utilisateur et visualisation canvas
 * Canvas 2D avec patte, cible, grille, et affichage d'angles
 */

import { calculateIK, applyServoConfig, clampTarget, isReachable } from "./ik.js";
import { ROBOT_CONFIG, SERVO_CONFIG, getServoConfig } from "./config.js";

const { L1_FEMUR, L2_TIBIA } = ROBOT_CONFIG;

// État global de l'UI
let uiState = {
  selectedLeg: "LF",
  targetX: 40,
  targetY: -30,
  elbowUp: false,
  currentIK: null,
  isDragging: false,
};

// Références DOM
let canvas, ctx, canvasContainer;

/**
 * Initialise le canvas et les contrôles
 */
export function initUI() {
  // Récupérer les éléments
  canvas = document.getElementById("ikCanvas");
  ctx = canvas.getContext("2d");
  canvasContainer = document.getElementById("canvasContainer");

  // Redimensionner le canvas
  resizeCanvas();
  window.addEventListener("resize", resizeCanvas);

  // Event listeners pour le canvas
  canvas.addEventListener("mousedown", onCanvasMouseDown);
  canvas.addEventListener("mousemove", onCanvasMouseMove);
  canvas.addEventListener("mouseup", onCanvasMouseUp);
  canvas.addEventListener("mouseleave", onCanvasMouseUp);
  canvas.addEventListener("wheel", onCanvasWheel);

  // Event listeners pour les contrôles
  document.getElementById("legSelect")?.addEventListener("change", onLegChanged);
  document.getElementById("elbowUpToggle")?.addEventListener("change", onElbowToggled);
  document.getElementById("sendNeutralBtn")?.addEventListener("click", onSendNeutral);
  document.getElementById("sendCurrentBtn")?.addEventListener("click", onSendCurrent);
  document.getElementById("simulationToggle")?.addEventListener("change", onSimulationToggled);

  // Charger la configuration depuis localStorage si disponible
  loadUIState();

  // Premier rendu
  redrawCanvas();

  console.log("✓ UI Initialisée");
}

/**
 * Redimensionne le canvas
 */
function resizeCanvas() {
  const rect = canvasContainer.getBoundingClientRect();
  canvas.width = rect.width;
  canvas.height = rect.height;
  redrawCanvas();
}

/**
 * Rendu complet du canvas
 */
export function redrawCanvas() {
  // Fond blanc
  ctx.fillStyle = "#fff";
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  // Grille et axes
  drawGrid();
  drawAxes();

  // Patte (L1 + L2)
  uiState.currentIK = calculateIK(uiState.targetX, uiState.targetY, uiState.elbowUp);
  drawLeg(uiState.currentIK);

  // Cible (point draggable)
  drawTarget();

  // Texte: angles et infos
  drawInfo();
}

/**
 * Dessine la grille (5mm)
 */
function drawGrid() {
  const scale = getCanvasScale();
  const gridStep = 5; // mm
  const gridPixels = gridStep * scale;

  ctx.strokeStyle = "#e0e0e0";
  ctx.lineWidth = 0.5;

  // Grille verticale
  for (let x = 0; x < canvas.width; x += gridPixels) {
    ctx.beginPath();
    ctx.moveTo(x, 0);
    ctx.lineTo(x, canvas.height);
    ctx.stroke();
  }

  // Grille horizontale
  for (let y = 0; y < canvas.height; y += gridPixels) {
    ctx.beginPath();
    ctx.moveTo(0, y);
    ctx.lineTo(canvas.width, y);
    ctx.stroke();
  }
}

/**
 * Dessine les axes X/Y
 */
function drawAxes() {
  const origin = getCanvasOrigin();
  const scale = getCanvasScale();
  const maxDist = 100; // mm

  // Axe X (rouge)
  ctx.strokeStyle = "#ff4444";
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.moveTo(origin.x - maxDist * scale, origin.y);
  ctx.lineTo(origin.x + maxDist * scale, origin.y);
  ctx.stroke();

  // Axe Y (bleu)
  ctx.strokeStyle = "#4444ff";
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.moveTo(origin.x, origin.y - maxDist * scale);
  ctx.lineTo(origin.x, origin.y + maxDist * scale);
  ctx.stroke();

  // Origine (0,0)
  ctx.fillStyle = "#000";
  ctx.beginPath();
  ctx.arc(origin.x, origin.y, 4, 0, 2 * Math.PI);
  ctx.fill();

  // Labels
  ctx.fillStyle = "#666";
  ctx.font = "12px Arial";
  ctx.fillText("X", origin.x + 60 * scale + 5, origin.y - 5);
  ctx.fillText("Y", origin.x - 10, origin.y - 60 * scale - 5);
}

/**
 * Dessine la patte (segments L1 et L2)
 * @param {object} ikResult - Résultat du calcul IK
 */
function drawLeg(ikResult) {
  const origin = getCanvasOrigin();
  const scale = getCanvasScale();

  const hipDeg = ikResult.hipDeg;
  const kneeDeg = ikResult.kneeDeg;

  // Angle hanche en radians
  const hipRad = (hipDeg * Math.PI) / 180;

  // Position genou (fin du segment L1)
  const kneeX = origin.x + L1_FEMUR * Math.cos(hipRad) * scale;
  const kneeY = origin.y + L1_FEMUR * Math.sin(hipRad) * scale;

  // Angle pied (hanche + genou, en radians)
  const kneeRad = (kneeDeg * Math.PI) / 180;
  const footAngle = hipRad + kneeRad;

  const footX = kneeX + L2_TIBIA * Math.cos(footAngle) * scale;
  const footY = kneeY + L2_TIBIA * Math.sin(footAngle) * scale;

  // Segment L1 (fémur) - bleu
  ctx.strokeStyle = "#0066ff";
  ctx.lineWidth = 8;
  ctx.beginPath();
  ctx.moveTo(origin.x, origin.y);
  ctx.lineTo(kneeX, kneeY);
  ctx.stroke();

  // Segment L2 (tibia) - vert
  ctx.strokeStyle = "#00cc00";
  ctx.lineWidth = 8;
  ctx.beginPath();
  ctx.moveTo(kneeX, kneeY);
  ctx.lineTo(footX, footY);
  ctx.stroke();

  // Articulations (cercles)
  ctx.fillStyle = "#ff9900";
  ctx.beginPath();
  ctx.arc(origin.x, origin.y, 6, 0, 2 * Math.PI);
  ctx.fill();

  ctx.fillStyle = "#cc00ff";
  ctx.beginPath();
  ctx.arc(kneeX, kneeY, 6, 0, 2 * Math.PI);
  ctx.fill();

  ctx.fillStyle = "#ff0066";
  ctx.beginPath();
  ctx.arc(footX, footY, 6, 0, 2 * Math.PI);
  ctx.fill();
}

/**
 * Dessine la cible (point draggable)
 */
function drawTarget() {
  const origin = getCanvasOrigin();
  const scale = getCanvasScale();

  const targetPixelX = origin.x + uiState.targetX * scale;
  const targetPixelY = origin.y + uiState.targetY * scale;

  // Cercle cible
  ctx.strokeStyle = uiState.isDragging ? "#ff0000" : "#ffaa00";
  ctx.lineWidth = 3;
  ctx.beginPath();
  ctx.arc(targetPixelX, targetPixelY, 10, 0, 2 * Math.PI);
  ctx.stroke();

  // Croix
  ctx.strokeStyle = "#ffaa00";
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.moveTo(targetPixelX - 5, targetPixelY);
  ctx.lineTo(targetPixelX + 5, targetPixelY);
  ctx.moveTo(targetPixelX, targetPixelY - 5);
  ctx.lineTo(targetPixelX, targetPixelY + 5);
  ctx.stroke();
}

/**
 * Dessine les informations (angles, distance, état)
 */
function drawInfo() {
  const ik = uiState.currentIK;
  const scale = 14;

  ctx.fillStyle = "#333";
  ctx.font = `${scale}px monospace`;
  ctx.textBaseline = "top";

  let y = 10;
  const lineHeight = scale + 4;

  // Patte sélectionnée
  ctx.fillText(`📍 Patte: ${uiState.selectedLeg}`, 10, y);
  y += lineHeight;

  // Position cible
  ctx.fillText(`🎯 Cible: (${ik.targetX?.toFixed(1) || uiState.targetX.toFixed(1)}, ${uiState.targetY.toFixed(1)}) mm`, 10, y);
  y += lineHeight;

  // Distance
  ctx.fillStyle = "#0066ff";
  ctx.fillText(`📏 Distance: ${ik.distanceMM.toFixed(1)} mm`, 10, y);
  y += lineHeight;

  // Angles IK
  ctx.fillStyle = "#00aa00";
  ctx.fillText(`📐 HIP:  ${ik.hipDeg.toFixed(1)}° (${ik.hipRad.toFixed(3)} rad)`, 10, y);
  y += lineHeight;

  ctx.fillStyle = "#00aa00";
  ctx.fillText(`📐 KNEE: ${ik.kneeDeg.toFixed(1)}° (${ik.kneeRad.toFixed(3)} rad)`, 10, y);
  y += lineHeight;

  // État
  const stateColors = { ok: "#00cc00", clamp: "#ffaa00", unreachable: "#ff3333" };
  ctx.fillStyle = stateColors[ik.state] || "#999";
  ctx.fillText(`⚠️  État: ${ik.state}`, 10, y);
  if (ik.message) {
    y += lineHeight;
    ctx.fillText(`   ${ik.message}`, 10, y);
  }
  y += lineHeight * 1.5;

  // Mode elbow
  ctx.fillStyle = "#333";
  ctx.fillText(`🦵 Mode: ${uiState.elbowUp ? "Elbow-UP" : "Elbow-DOWN"}`, 10, y);
}

/**
 * Event: Changement de patte sélectionnée
 */
function onLegChanged(e) {
  uiState.selectedLeg = e.target.value;
  updateServoConfigPanel();
  saveUIState();
  redrawCanvas();
}

/**
 * Event: Toggle elbow-up/down
 */
function onElbowToggled(e) {
  uiState.elbowUp = e.target.checked;
  saveUIState();
  redrawCanvas();
  updateServoPanelValues();
}

/**
 * Event: Clic souris sur canvas
 */
function onCanvasMouseDown(e) {
  const { x, y } = getMousePositionMM(e);
  const dx = x - uiState.targetX;
  const dy = y - uiState.targetY;
  const dist = Math.sqrt(dx * dx + dy * dy);

  if (dist < 15) {
    // Dans le rayon de la cible
    uiState.isDragging = true;
  }
}

/**
 * Event: Déplacement souris
 */
function onCanvasMouseMove(e) {
  if (!uiState.isDragging) return;

  const { x, y } = getMousePositionMM(e);
  const clamped = clampTarget(x, y);

  uiState.targetX = clamped.x;
  uiState.targetY = clamped.y;

  saveUIState();
  redrawCanvas();
  updateServoPanelValues();

  // Trigger event pour les listeners externes
  document.dispatchEvent(new CustomEvent("targetChanged", { detail: { x: uiState.targetX, y: uiState.targetY } }));
}

/**
 * Event: Relâchement souris
 */
function onCanvasMouseUp() {
  if (uiState.isDragging) {
    uiState.isDragging = false;
    redrawCanvas();
  }
}

/**
 * Event: Molette souris (zoom)
 */
function onCanvasWheel(e) {
  e.preventDefault();
  // Implémentation du zoom si souhaité
}

/**
 * Event: Bouton "Send Neutral"
 */
function onSendNeutral() {
  document.dispatchEvent(new CustomEvent("sendNeutral", { detail: { leg: uiState.selectedLeg } }));
}

/**
 * Event: Bouton "Send Current"
 */
function onSendCurrent() {
  document.dispatchEvent(new CustomEvent("sendCurrent", { detail: { leg: uiState.selectedLeg } }));
}

/**
 * Event: Toggle simulation mode
 */
function onSimulationToggled(e) {
  const isSimulation = e.target.checked;
  document.dispatchEvent(new CustomEvent("simulationToggled", { detail: { simulation: isSimulation } }));
}

/**
 * Met à jour le panneau de configuration servo
 */
export function updateServoConfigPanel() {
  const leg = uiState.selectedLeg;
  const hipConfig = getServoConfig(leg, "HIP");
  const kneeConfig = getServoConfig(leg, "KNEE");

  if (!hipConfig || !kneeConfig) return;

  // HIP
  document.getElementById("hipOffsetInput").value = hipConfig.offsetDeg;
  document.getElementById("hipInvertCheck").checked = hipConfig.invert;
  document.getElementById("hipMinInput").value = hipConfig.minDeg;
  document.getElementById("hipMaxInput").value = hipConfig.maxDeg;

  // KNEE
  document.getElementById("kneeOffsetInput").value = kneeConfig.offsetDeg;
  document.getElementById("kneeInvertCheck").checked = kneeConfig.invert;
  document.getElementById("kneeMinInput").value = kneeConfig.minDeg;
  document.getElementById("kneeMaxInput").value = kneeConfig.maxDeg;

  updateServoPanelValues();
}

/**
 * Met à jour les valeurs d'angle servo affichées
 */
export function updateServoPanelValues() {
  if (!uiState.currentIK) return;

  const leg = uiState.selectedLeg;
  const hipConfig = getServoConfig(leg, "HIP");
  const kneeConfig = getServoConfig(leg, "KNEE");

  if (!hipConfig || !kneeConfig) return;

  const hipServoDeg = applyServoConfig(uiState.currentIK.hipDeg, hipConfig);
  const kneeServoDeg = applyServoConfig(uiState.currentIK.kneeDeg, kneeConfig);

  document.getElementById("hipOutputValue").textContent = hipServoDeg.toFixed(1);
  document.getElementById("kneeOutputValue").textContent = kneeServoDeg.toFixed(1);
}

/**
 * Obtient la position de la souris en coordonnées canvas (mm)
 */
function getMousePositionMM(e) {
  const rect = canvas.getBoundingClientRect();
  const pixelX = e.clientX - rect.left;
  const pixelY = e.clientY - rect.top;

  const origin = getCanvasOrigin();
  const scale = getCanvasScale();

  const x = (pixelX - origin.x) / scale;
  const y = (pixelY - origin.y) / scale;

  return { x, y };
}

/**
 * Retourne l'origine du canvas (centre)
 */
function getCanvasOrigin() {
  return {
    x: canvas.width / 2,
    y: canvas.height / 2,
  };
}

/**
 * Retourne l'échelle pixels/mm
 */
function getCanvasScale() {
  return Math.min(canvas.width, canvas.height) / 150; // ~150mm de portée = hauteur/largeur
}

/**
 * Sauvegarde l'état de l'UI en localStorage
 */
function saveUIState() {
  localStorage.setItem(
    "uiState",
    JSON.stringify({
      selectedLeg: uiState.selectedLeg,
      targetX: uiState.targetX,
      targetY: uiState.targetY,
      elbowUp: uiState.elbowUp,
    })
  );
}

/**
 * Charge l'état de l'UI depuis localStorage
 */
function loadUIState() {
  const saved = localStorage.getItem("uiState");
  if (saved) {
    try {
      const loaded = JSON.parse(saved);
      uiState = { ...uiState, ...loaded };
      
      // Mettre à jour les contrôles
      document.getElementById("legSelect").value = uiState.selectedLeg;
      document.getElementById("elbowUpToggle").checked = uiState.elbowUp;
      
      updateServoConfigPanel();
    } catch (e) {
      console.warn("Failed to load UI state", e);
    }
  }
}

/**
 * Exporte l'état pour utilisation externe
 */
export function getUIState() {
  return { ...uiState };
}

/**
 * Définit la cible (pour utilisation externe)
 */
export function setTarget(x, y) {
  uiState.targetX = x;
  uiState.targetY = y;
  saveUIState();
  redrawCanvas();
  updateServoPanelValues();
}
