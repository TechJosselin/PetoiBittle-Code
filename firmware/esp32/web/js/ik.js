/**
 * ik.js - Calcul inverse cinématique 2DOF (hanche + genou)
 * Plan sagittal uniquement (X, Y)
 */

import { ROBOT_CONFIG } from "./config.js";

const { L1_FEMUR, L2_TIBIA, MIN_REACH, MAX_REACH } = ROBOT_CONFIG;

/**
 * Résultat du calcul IK
 * @typedef {object} IKResult
 * @property {number} hipDeg - Angle hanche en degrés (servo: 0-180)
 * @property {number} kneeDeg - Angle genou en degrés (servo: 0-180)
 * @property {number} hipRad - Angle hanche en radians
 * @property {number} kneeRad - Angle genou en radians
 * @property {number} distanceMM - Distance pied->hanche
 * @property {string} state - "ok", "clamp", "unreachable"
 * @property {string} message - Description de l'état
 */

/**
 * Calcule la cinématique inverse pour une cible (x, y)
 * @param {number} x - Position X du pied (mm)
 * @param {number} y - Position Y du pied (mm)
 * @param {boolean} elbowUp - Utiliser la configuration elbow-up (false = elbow-down par défaut)
 * @returns {IKResult} Angles calculés et état
 */
export function calculateIK(x, y, elbowUp = false) {
  let state = "ok";
  let message = "";

  // Calcul de la distance
  let r = Math.sqrt(x * x + y * y);

  // Vérification des limites de portée
  if (r < MIN_REACH - 1) {
    state = "clamp";
    message = `Distance ${r.toFixed(1)}mm < portée min (${MIN_REACH}mm)`;
    r = MIN_REACH;
  }
  if (r > MAX_REACH + 1) {
    state = "clamp";
    message = `Distance ${r.toFixed(1)}mm > portée max (${MAX_REACH}mm)`;
    r = MAX_REACH;
  }

  // Calcul inverse cinématique
  let hipRad, kneeRad;

  try {
    // Loi des cosinus pour angle interne du genou
    const cosK =
      (r * r - L1_FEMUR * L1_FEMUR - L2_TIBIA * L2_TIBIA) /
      (2 * L1_FEMUR * L2_TIBIA);

    // Clamp cosK pour éviter les NaN
    let cosKClamped = Math.max(-1, Math.min(1, cosK));

    if (Math.abs(cosKClamped - cosK) > 0.01) {
      state = "clamp";
      message = `Angle genou clampé (cosK=${cosK.toFixed(3)})`;
    }

    // Angle interne du genou
    kneeRad = Math.acos(cosKClamped);

    // Angle d'approche (atan2 de la cible)
    const A = Math.atan2(y, x);

    // Angle offset due à L2 et angle du genou
    const sinK = Math.sin(kneeRad);
    const cosKForB = Math.cos(kneeRad);
    const B = Math.atan2(L2_TIBIA * sinK, L1_FEMUR + L2_TIBIA * cosKForB);

    // Calcul de l'angle hanche
    if (elbowUp) {
      // Elbow-up: HIP = A + B
      hipRad = A + B;
    } else {
      // Elbow-down (défaut): HIP = A - B
      hipRad = A - B;
    }

    // Normaliser les angles
    hipRad = normalizeAngle(hipRad);

    // Pour elbow-up, le genou doit être négatif
    if (elbowUp) {
      kneeRad = -kneeRad;
    }
  } catch (e) {
    console.error("IK Calculation error:", e);
    state = "unreachable";
    message = `Erreur calcul IK: ${e.message}`;
    hipRad = 0;
    kneeRad = 0;
  }

  // Conversion en degrés
  const hipDeg = hipRad * (180 / Math.PI);
  const kneeDeg = kneeRad * (180 / Math.PI);

  return {
    hipDeg,
    kneeDeg,
    hipRad,
    kneeRad,
    distanceMM: r,
    state,
    message,
  };
}

/**
 * Applique les offsets et inversions pour obtenir l'angle servo final
 * @param {number} ikDeg - Angle IK en degrés
 * @param {object} servoConfig - {offsetDeg, invert, minDeg, maxDeg}
 * @returns {number} Angle servo final (0-180, clampé)
 */
export function applyServoConfig(ikDeg, servoConfig) {
  let servoDeg = ikDeg;

  // Appliquer l'inversion
  if (servoConfig.invert) {
    servoDeg = 180 - servoDeg;
  }

  // Appliquer l'offset
  servoDeg += servoConfig.offsetDeg;

  // Clamp aux limites du servo
  servoDeg = Math.max(servoConfig.minDeg, Math.min(servoConfig.maxDeg, servoDeg));

  return servoDeg;
}

/**
 * Normalise un angle en radians à [-π, π]
 * @param {number} angle - Angle en radians
 * @returns {number} Angle normalisé
 */
function normalizeAngle(angle) {
  while (angle > Math.PI) angle -= 2 * Math.PI;
  while (angle < -Math.PI) angle += 2 * Math.PI;
  return angle;
}

/**
 * Valide si une cible est atteignable
 * @param {number} x - Position X (mm)
 * @param {number} y - Position Y (mm)
 * @returns {boolean} true si la cible est dans la zone de portée
 */
export function isReachable(x, y) {
  const r = Math.sqrt(x * x + y * y);
  return r >= MIN_REACH - 1 && r <= MAX_REACH + 1;
}

/**
 * Convertit un point cartésien en point atteignable le plus proche
 * @param {number} x - Position X (mm)
 * @param {number} y - Position Y (mm)
 * @returns {object} {x, y} point clampé
 */
export function clampTarget(x, y) {
  const r = Math.sqrt(x * x + y * y);
  const angle = Math.atan2(y, x);

  let rClamped = Math.max(MIN_REACH, Math.min(MAX_REACH, r));

  return {
    x: rClamped * Math.cos(angle),
    y: rClamped * Math.sin(angle),
  };
}
