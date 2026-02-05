/**
 * config.js - Configuration des servos et mapping PCA9685
 * Gestion des offsets, inversions, limites par servo
 */

// Dimensions physiques (mm)
export const ROBOT_CONFIG = {
  L1_FEMUR: 46,    // Longueur fémur (hanche->genou)
  L2_TIBIA: 21,    // Longueur tibia (genou->pied)
  MIN_REACH: 25,   // Portée min (|L1-L2|)
  MAX_REACH: 67,   // Portée max (L1+L2)
};

// Mapping des canaux PCA9685
export const SERVO_MAPPING = {
  LF: { HIP: 0, KNEE: 1 },
  LR: { HIP: 2, KNEE: 3 },
  RF: { HIP: 4, KNEE: 5 },
  RR: { HIP: 6, KNEE: 7 },
};

// Configuration par servo (offset, inversion, limites)
// Note: Ces valeurs sont à calibrer selon votre robot
export const SERVO_CONFIG = {
  LF_HIP: {
    name: "LF_HIP",
    pcaChannel: 0,
    offsetDeg: 0,
    invert: false,
    minDeg: 0,
    maxDeg: 180,
  },
  LF_KNEE: {
    name: "LF_KNEE",
    pcaChannel: 1,
    offsetDeg: 0,
    invert: false,
    minDeg: 0,
    maxDeg: 180,
  },
  LR_HIP: {
    name: "LR_HIP",
    pcaChannel: 2,
    offsetDeg: 0,
    invert: false,
    minDeg: 0,
    maxDeg: 180,
  },
  LR_KNEE: {
    name: "LR_KNEE",
    pcaChannel: 3,
    offsetDeg: 0,
    invert: false,
    minDeg: 0,
    maxDeg: 180,
  },
  RF_HIP: {
    name: "RF_HIP",
    pcaChannel: 4,
    offsetDeg: 0,
    invert: false,
    minDeg: 0,
    maxDeg: 180,
  },
  RF_KNEE: {
    name: "RF_KNEE",
    pcaChannel: 5,
    offsetDeg: 0,
    invert: false,
    minDeg: 0,
    maxDeg: 180,
  },
  RR_HIP: {
    name: "RR_HIP",
    pcaChannel: 6,
    offsetDeg: 0,
    invert: false,
    minDeg: 0,
    maxDeg: 180,
  },
  RR_KNEE: {
    name: "RR_KNEE",
    pcaChannel: 7,
    offsetDeg: 0,
    invert: false,
    minDeg: 0,
    maxDeg: 180,
  },
};

/**
 * Récupère la configuration servo pour une patte donnée
 * @param {string} legName - LF, LR, RF, RR
 * @param {string} jointName - HIP ou KNEE
 * @returns {object} Configuration servo
 */
export function getServoConfig(legName, jointName) {
  const servoName = `${legName}_${jointName}`;
  return SERVO_CONFIG[servoName] || null;
}

/**
 * Récupère le canal PCA pour une patte/articulation
 * @param {string} legName - LF, LR, RF, RR
 * @param {string} jointName - HIP ou KNEE
 * @returns {number} Numéro de canal PCA9685
 */
export function getPCAChannel(legName, jointName) {
  return SERVO_MAPPING[legName]?.[jointName] ?? null;
}

/**
 * Met à jour la configuration d'un servo
 * @param {string} servoName - e.g., "LF_HIP"
 * @param {object} updates - {offsetDeg, invert, minDeg, maxDeg}
 */
export function updateServoConfig(servoName, updates) {
  if (SERVO_CONFIG[servoName]) {
    Object.assign(SERVO_CONFIG[servoName], updates);
    // Sauvegarder en localStorage pour persistance
    localStorage.setItem("servoConfig", JSON.stringify(SERVO_CONFIG));
  }
}

/**
 * Charge la configuration depuis localStorage
 */
export function loadServoConfig() {
  const saved = localStorage.getItem("servoConfig");
  if (saved) {
    try {
      const loaded = JSON.parse(saved);
      Object.assign(SERVO_CONFIG, loaded);
    } catch (e) {
      console.warn("Failed to load servo config from localStorage", e);
    }
  }
}

/**
 * Réinitialise à la configuration par défaut
 */
export function resetServoConfig() {
  localStorage.removeItem("servoConfig");
  window.location.reload();
}
