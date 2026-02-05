#pragma once

/**
 * ik_page.h - Page HTML IK complète avec CSS et JS inline
 * Version 2 avec support mobile et animation loop
 */

static const char* HTML_IK_PAGE = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no, viewport-fit=cover">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <title>🤖 Bittle Robot IK Control</title>
    <style>
* {margin:0;padding:0;box-sizing:border-box}
html,body {width:100%;height:100%;touch-action:none}
body {font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Oxygen,Ubuntu,Cantarell,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#333;min-height:100vh;padding:20px}
.container {max-width:1400px;margin:0 auto;background:#fff;border-radius:12px;box-shadow:0 20px 60px rgba(0,0,0,0.3);overflow:hidden}
.header {background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff;padding:30px;border-bottom:4px solid #ffaa00}
.header h1 {font-size:2.2em;margin-bottom:15px;text-shadow:2px 2px 4px rgba(0,0,0,0.3)}
.status-bar {display:flex;gap:30px;flex-wrap:wrap;font-size:0.95em}
.status-item {background:rgba(255,255,255,0.2);padding:8px 12px;border-radius:6px;border-left:3px solid #ffaa00}
.main-layout {display:grid;grid-template-columns:1.5fr 1fr;gap:30px;padding:30px;min-height:600px}
@media (max-width:1100px){.main-layout{grid-template-columns:1fr}}
.ik-section {display:flex;flex-direction:column;gap:15px}
.section-header {display:flex;justify-content:space-between;align-items:center;flex-wrap:wrap;gap:20px}
.section-header h2 {font-size:1.5em;color:#667eea}
.controls-inline {display:flex;gap:15px;align-items:center;flex-wrap:wrap}
.controls-inline label {font-weight:500;color:#555}
.controls-inline select {padding:6px 10px;border:2px solid #ddd;border-radius:4px;font-size:1em;cursor:pointer}
.controls-inline select:hover,.controls-inline select:focus {border-color:#667eea;outline:0}
.controls-inline input[type="checkbox"] {width:18px;height:18px;cursor:pointer;accent-color:#667eea}
.canvas-container {flex:1;display:flex;align-items:center;justify-content:center;background:#f9f9f9;border:2px solid #e0e0e0;border-radius:8px;overflow:hidden;min-height:500px;touch-action:none;position:relative}
#ikCanvas {display:block;width:100%;height:100%;cursor:grab;touch-action:none;background:#fff}
#ikCanvas:active {cursor:grabbing}
.canvas-info {background:#f0f4ff;padding:12px;border-radius:6px;font-size:0.9em;color:#555;border-left:4px solid #667eea}
.config-section {background:#f9f9f9;padding:20px;border-radius:8px;border:2px solid #e0e0e0}
.config-section h2 {font-size:1.3em;color:#667eea;margin-bottom:20px;padding-bottom:10px;border-bottom:2px solid #ddd}
.servo-config-panel {background:#fff;padding:15px;border-radius:6px;margin-bottom:15px;border:1px solid #ddd}
.servo-config-panel h3 {color:#764ba2;margin-bottom:12px;font-size:1.1em}
.config-grid {display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-bottom:10px}
@media (max-width:600px){.config-grid{grid-template-columns:1fr}}
.config-input {display:flex;flex-direction:column;gap:5px}
.config-input label {font-size:0.9em;font-weight:500;color:#555}
.config-input input {padding:6px 8px;border:1px solid #ddd;border-radius:4px;font-size:0.95em}
.config-input input:focus {border-color:#667eea;outline:0;box-shadow:0 0 0 3px rgba(102,126,234,0.1)}
.servo-output {background:#f0f4ff;padding:8px 12px;border-radius:4px;font-weight:600;color:#667eea;font-size:0.95em;border-left:3px solid #667eea}
.button-group {display:flex;gap:10px;margin-top:20px}
.btn {flex:1;padding:12px 20px;border:0;border-radius:6px;font-size:1em;font-weight:600;cursor:pointer;transition:all 0.3s;text-align:center;min-height:44px}
.btn-primary {background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:#fff}
.btn-primary:hover {transform:translateY(-2px);box-shadow:0 10px 20px rgba(102,126,234,0.3)}
.btn-secondary {background:#ffaa00;color:#fff}
.btn-secondary:hover {background:#ff9500;transform:translateY(-2px);box-shadow:0 10px 20px rgba(255,170,0,0.3)}
    </style>
</head>
<body>
<div class="container">
<header class="header">
    <h1>🤖 Bittle Robot - Contrôle Inverse Cinématique</h1>
    <div class="status-bar">
        <div id="networkStatus" class="status-item">📡 État réseau...</div>
        <div id="platformStatus" class="status-item">Platform: ESP32-C6</div>
        <div id="ledStatus" class="status-item">LED: OFF</div>
        <div id="imuStatus" class="status-item">IMU: --</div>
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
        <label><input type="checkbox" id="elbowUpToggle"> Elbow-UP</label>
    </div>
</div>
<div id="canvasContainer" class="canvas-container">
    <canvas id="ikCanvas" width="600" height="500"></canvas>
</div>
<div class="canvas-info">
    <p>🖱️ Drag la cible (⭕ orange) pour modifier la position du pied.</p>
</div>
</section>

<section class="config-section">
<h2>⚙️ Configuration Servo</h2>
<div class="servo-config-panel">
    <h3>HIP (Hanche)</h3>
    <div class="config-grid">
        <div class="config-input">
            <label for="hipOffsetInput">Offset (°):</label>
            <input type="number" id="hipOffsetInput" value="0" step="1" min="-90" max="90">
        </div>
        <div class="config-input">
            <label><input type="checkbox" id="hipInvertCheck"> Inverser</label>
        </div>
        <div class="config-input">
            <label for="hipMinInput">Min (°):</label>
            <input type="number" id="hipMinInput" value="0" step="1" min="0" max="270">
        </div>
        <div class="config-input">
            <label for="hipMaxInput">Max (°):</label>
            <input type="number" id="hipMaxInput" value="270" step="1" min="0" max="270">
        </div>
        <div class="config-input">
            <label for="hipMinPulseInput">Min Pulse (µs):</label>
            <input type="number" id="hipMinPulseInput" value="500" step="1" min="250" max="3000">
        </div>
        <div class="config-input">
            <label for="hipMaxPulseInput">Max Pulse (µs):</label>
            <input type="number" id="hipMaxPulseInput" value="2500" step="1" min="250" max="3000">
        </div>
    </div>
    <div class="servo-output">Angle Servo: <span id="hipOutputValue">90.0</span>°</div>
</div>
<div class="servo-config-panel">
    <h3>KNEE (Genou)</h3>
    <div class="config-grid">
        <div class="config-input">
            <label for="kneeOffsetInput">Offset (°):</label>
            <input type="number" id="kneeOffsetInput" value="0" step="1" min="-90" max="90">
        </div>
        <div class="config-input">
            <label><input type="checkbox" id="kneeInvertCheck"> Inverser</label>
        </div>
        <div class="config-input">
            <label for="kneeMinInput">Min (°):</label>
            <input type="number" id="kneeMinInput" value="0" step="1" min="0" max="270">
        </div>
        <div class="config-input">
            <label for="kneeMaxInput">Max (°):</label>
            <input type="number" id="kneeMaxInput" value="270" step="1" min="0" max="270">
        </div>
        <div class="config-input">
            <label for="kneeMinPulseInput">Min Pulse (µs):</label>
            <input type="number" id="kneeMinPulseInput" value="500" step="1" min="250" max="3000">
        </div>
        <div class="config-input">
            <label for="kneeMaxPulseInput">Max Pulse (µs):</label>
            <input type="number" id="kneeMaxPulseInput" value="2500" step="1" min="250" max="3000">
        </div>
    </div>
    <div class="servo-output">Angle Servo: <span id="kneeOutputValue">90.0</span>°</div>
</div>
<div class="button-group">
    <button id="sendNeutralBtn" class="btn btn-secondary">🔄 Neutre (90°)</button>
    <button id="sendCurrentBtn" class="btn btn-primary">📤 Envoyer Angles</button>
</div>
</section>
</div>
</div>

<script>
const ROBOT_CONFIG={L1_FEMUR:46,L2_TIBIA:21};
const SERVO_CONFIG={
LF_HIP:{name:"LF_HIP",pcaChannel:0,offsetDeg:0,invert:false,minDeg:0,maxDeg:270,minPulseUs:500,maxPulseUs:2500},
LF_KNEE:{name:"LF_KNEE",pcaChannel:1,offsetDeg:0,invert:false,minDeg:0,maxDeg:270,minPulseUs:500,maxPulseUs:2500},
LR_HIP:{name:"LR_HIP",pcaChannel:2,offsetDeg:0,invert:false,minDeg:0,maxDeg:270,minPulseUs:500,maxPulseUs:2500},
LR_KNEE:{name:"LR_KNEE",pcaChannel:3,offsetDeg:0,invert:false,minDeg:0,maxDeg:270,minPulseUs:500,maxPulseUs:2500},
RF_HIP:{name:"RF_HIP",pcaChannel:4,offsetDeg:0,invert:false,minDeg:0,maxDeg:270,minPulseUs:500,maxPulseUs:2500},
RF_KNEE:{name:"RF_KNEE",pcaChannel:5,offsetDeg:0,invert:false,minDeg:0,maxDeg:270,minPulseUs:500,maxPulseUs:2500},
RR_HIP:{name:"RR_HIP",pcaChannel:6,offsetDeg:0,invert:false,minDeg:0,maxDeg:270,minPulseUs:500,maxPulseUs:2500},
RR_KNEE:{name:"RR_KNEE",pcaChannel:7,offsetDeg:0,invert:false,minDeg:0,maxDeg:270,minPulseUs:500,maxPulseUs:2500}
};
let uiState={selectedLeg:"LF",targetX:40,targetY:-30,elbowUp:false,isDragging:false};
let canvas,ctx,animationId;

function calculateIK(x,y,elbowUp){
const r=Math.sqrt(x*x+y*y);const L1=ROBOT_CONFIG.L1_FEMUR,L2=ROBOT_CONFIG.L2_TIBIA;
const cosK=(r*r-L1*L1-L2*L2)/(2*L1*L2);const cosKClamped=Math.max(-1,Math.min(1,cosK));
const kneeRad=Math.acos(cosKClamped);const A=Math.atan2(y,x);
const sinK=Math.sin(kneeRad),cosKForB=Math.cos(kneeRad);const B=Math.atan2(L2*sinK,L1+L2*cosKForB);
let hipRad=elbowUp?A+B:A-B;
return{hipDeg:hipRad*180/Math.PI,kneeDeg:kneeRad*180/Math.PI,hipRad,kneeRad,distanceMM:r};
}

function applyServoConfig(ikDeg,servoConfig){
let servoDeg=ikDeg;if(servoConfig.invert)servoDeg=180-servoDeg;servoDeg+=servoConfig.offsetDeg;
return Math.max(servoConfig.minDeg,Math.min(servoConfig.maxDeg,servoDeg));
}

function getServoConfig(leg,joint){const name=`${leg}_${joint}`;return SERVO_CONFIG[name]||null;}

function getMousePosMM(e){const rect=canvas.getBoundingClientRect();const px=e.clientX-rect.left;const py=e.clientY-rect.top;
const origin={x:canvas.width/2,y:canvas.height/2};const scale=Math.min(canvas.width,canvas.height)/150;
return{x:(px-origin.x)/scale,y:(py-origin.y)/scale};}

function getTouchPosMM(touch){const rect=canvas.getBoundingClientRect();const px=touch.clientX-rect.left;const py=touch.clientY-rect.top;
const origin={x:canvas.width/2,y:canvas.height/2};const scale=Math.min(canvas.width,canvas.height)/150;
return{x:(px-origin.x)/scale,y:(py-origin.y)/scale};}

function updateTarget(pos){const r=Math.sqrt(pos.x*pos.x+pos.y*pos.y);const angle=Math.atan2(pos.y,pos.x);
let rClamped=Math.max(25,Math.min(67,r));uiState.targetX=rClamped*Math.cos(angle);uiState.targetY=rClamped*Math.sin(angle);
updatePanelValues();autoSendServos();}

let autoSendTimeout=null;
function autoSendServos(){
clearTimeout(autoSendTimeout);autoSendTimeout=setTimeout(()=>{
const hip=parseFloat(document.getElementById("hipOutputValue").textContent);
const knee=parseFloat(document.getElementById("kneeOutputValue").textContent);
sendServos({leg:uiState.selectedLeg,angles:{hipDeg:hip,kneeDeg:knee}});},50);}

function redrawCanvas(){
ctx.fillStyle="#fff";ctx.fillRect(0,0,canvas.width,canvas.height);
const origin={x:canvas.width/2,y:canvas.height/2};const scale=Math.min(canvas.width,canvas.height)/150;
drawGrid(origin,scale);drawAxes(origin,scale);const ik=calculateIK(uiState.targetX,uiState.targetY,uiState.elbowUp);
drawLeg(ik,origin,scale);drawTarget(origin,scale);drawInfo(ik);
}

function drawGrid(origin,scale){ctx.strokeStyle="#e0e0e0";ctx.lineWidth=0.5;
for(let x=0;x<canvas.width;x+=5*scale){ctx.beginPath();ctx.moveTo(x,0);ctx.lineTo(x,canvas.height);ctx.stroke();}
for(let y=0;y<canvas.height;y+=5*scale){ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(canvas.width,y);ctx.stroke();}}

function drawAxes(origin,scale){
ctx.strokeStyle="#ff4444";ctx.lineWidth=2;ctx.beginPath();ctx.moveTo(origin.x-100*scale,origin.y);ctx.lineTo(origin.x+100*scale,origin.y);ctx.stroke();
ctx.strokeStyle="#4444ff";ctx.beginPath();ctx.moveTo(origin.x,origin.y-100*scale);ctx.lineTo(origin.x,origin.y+100*scale);ctx.stroke();
ctx.fillStyle="#000";ctx.beginPath();ctx.arc(origin.x,origin.y,4,0,2*Math.PI);ctx.fill();}

function drawLeg(ik,origin,scale){const L1=ROBOT_CONFIG.L1_FEMUR,L2=ROBOT_CONFIG.L2_TIBIA;
const hipRad=ik.hipDeg*Math.PI/180;const kneeX=origin.x+L1*Math.cos(hipRad)*scale;const kneeY=origin.y+L1*Math.sin(hipRad)*scale;
const kneeRad=ik.kneeRad;const footAngle=hipRad+kneeRad;const footX=kneeX+L2*Math.cos(footAngle)*scale;const footY=kneeY+L2*Math.sin(footAngle)*scale;
ctx.strokeStyle="#0066ff";ctx.lineWidth=8;ctx.beginPath();ctx.moveTo(origin.x,origin.y);ctx.lineTo(kneeX,kneeY);ctx.stroke();
ctx.strokeStyle="#00cc00";ctx.beginPath();ctx.moveTo(kneeX,kneeY);ctx.lineTo(footX,footY);ctx.stroke();
ctx.fillStyle="#ff9900";ctx.beginPath();ctx.arc(origin.x,origin.y,6,0,2*Math.PI);ctx.fill();
ctx.fillStyle="#cc00ff";ctx.beginPath();ctx.arc(kneeX,kneeY,6,0,2*Math.PI);ctx.fill();
ctx.fillStyle="#ff0066";ctx.beginPath();ctx.arc(footX,footY,6,0,2*Math.PI);ctx.fill();}

function drawTarget(origin,scale){const tx=origin.x+uiState.targetX*scale;const ty=origin.y+uiState.targetY*scale;
ctx.strokeStyle=uiState.isDragging?"#ff0000":"#ffaa00";ctx.lineWidth=3;ctx.beginPath();ctx.arc(tx,ty,10,0,2*Math.PI);ctx.stroke();}

function drawInfo(ik){ctx.fillStyle="#333";ctx.font="14px monospace";ctx.textBaseline="top";let y=10;
ctx.fillText(`📍 Patte: ${uiState.selectedLeg}`,10,y);y+=18;
ctx.fillText(`🎯 Cible: (${uiState.targetX.toFixed(1)}, ${uiState.targetY.toFixed(1)}) mm`,10,y);y+=18;
ctx.fillStyle="#0066ff";ctx.fillText(`📏 Distance: ${ik.distanceMM.toFixed(1)} mm`,10,y);y+=18;
ctx.fillStyle="#00aa00";ctx.fillText(`📐 HIP: ${ik.hipDeg.toFixed(1)}°`,10,y);y+=18;
ctx.fillText(`📐 KNEE: ${ik.kneeDeg.toFixed(1)}°`,10,y);}

function startAnimation(){
function animate(){redrawCanvas();animationId=requestAnimationFrame(animate);}animate();}

function updateConfigPanel(){const leg=uiState.selectedLeg;const hipConfig=getServoConfig(leg,"HIP");const kneeConfig=getServoConfig(leg,"KNEE");
if(hipConfig){document.getElementById("hipOffsetInput").value=hipConfig.offsetDeg;document.getElementById("hipInvertCheck").checked=hipConfig.invert;
document.getElementById("hipMinInput").value=hipConfig.minDeg;document.getElementById("hipMaxInput").value=hipConfig.maxDeg;
if(document.getElementById("hipMinPulseInput"))document.getElementById("hipMinPulseInput").value=hipConfig.minPulseUs||500;
if(document.getElementById("hipMaxPulseInput"))document.getElementById("hipMaxPulseInput").value=hipConfig.maxPulseUs||2500;
}
if(kneeConfig){document.getElementById("kneeOffsetInput").value=kneeConfig.offsetDeg;document.getElementById("kneeInvertCheck").checked=kneeConfig.invert;
document.getElementById("kneeMinInput").value=kneeConfig.minDeg;document.getElementById("kneeMaxInput").value=kneeConfig.maxDeg;
if(document.getElementById("kneeMinPulseInput"))document.getElementById("kneeMinPulseInput").value=kneeConfig.minPulseUs||500;
if(document.getElementById("kneeMaxPulseInput"))document.getElementById("kneeMaxPulseInput").value=kneeConfig.maxPulseUs||2500;
}
updatePanelValues();}

function updatePanelValues(){const hip=getServoConfig(uiState.selectedLeg,"HIP");const knee=getServoConfig(uiState.selectedLeg,"KNEE");
if(!hip||!knee)return;const offset={HIP:parseInt(document.getElementById("hipOffsetInput").value)||0,KNEE:parseInt(document.getElementById("kneeOffsetInput").value)||0};
const invert={HIP:document.getElementById("hipInvertCheck").checked,KNEE:document.getElementById("kneeInvertCheck").checked};
const minMax={HIP:{min:parseInt(document.getElementById("hipMinInput").value)||0,max:parseInt(document.getElementById("hipMaxInput").value)||180},
KNEE:{min:parseInt(document.getElementById("kneeMinInput").value)||0,max:parseInt(document.getElementById("kneeMaxInput").value)||180}};
const ik=calculateIK(uiState.targetX,uiState.targetY,uiState.elbowUp);let hipDeg=ik.hipDeg;let kneeDeg=ik.kneeDeg;
if(invert.HIP)hipDeg=180-hipDeg;hipDeg+=offset.HIP;hipDeg=Math.max(minMax.HIP.min,Math.min(minMax.HIP.max,hipDeg));
if(invert.KNEE)kneeDeg=180-kneeDeg;kneeDeg+=offset.KNEE;kneeDeg=Math.max(minMax.KNEE.min,Math.min(minMax.KNEE.max,kneeDeg));
document.getElementById("hipOutputValue").textContent=hipDeg.toFixed(1);document.getElementById("kneeOutputValue").textContent=kneeDeg.toFixed(1);}

function buildServoPayload(servoData){const leg=servoData.leg;const hip=getServoConfig(leg,"HIP");const knee=getServoConfig(leg,"KNEE");
if(!hip||!knee)throw new Error(`Invalid leg: ${leg}`);
const hipServoDeg=applyServoConfig(servoData.angles.hipDeg,hip);
const kneeServoDeg=applyServoConfig(servoData.angles.kneeDeg,knee);
// Read pulse calibration inputs (fallback to config defaults)
const hipMinPulse = parseInt(document.getElementById("hipMinPulseInput")?.value) || hip.minPulseUs || 500;
const hipMaxPulse = parseInt(document.getElementById("hipMaxPulseInput")?.value) || hip.maxPulseUs || 2500;
const kneeMinPulse = parseInt(document.getElementById("kneeMinPulseInput")?.value) || knee.minPulseUs || 500;
const kneeMaxPulse = parseInt(document.getElementById("kneeMaxPulseInput")?.value) || knee.maxPulseUs || 2500;
// Map 0-270 deg to pulse range
const hipPulse = Math.round(hipMinPulse + (hipServoDeg/270.0)*(hipMaxPulse-hipMinPulse));
const kneePulse = Math.round(kneeMinPulse + (kneeServoDeg/270.0)*(kneeMaxPulse-kneeMinPulse));
return{leg,targets:{x:0,y:0,unit:"mm"},angles:{hipDeg:servoData.angles.hipDeg,kneeDeg:servoData.angles.kneeDeg},
servos:[{name:hip.name,pcaChannel:hip.pcaChannel,deg:hipServoDeg,pulse_us:hipPulse},{name:knee.name,pcaChannel:knee.pcaChannel,deg:kneeServoDeg,pulse_us:kneePulse}]};}

async function sendServos(servoData){
    try {
        const payload = buildServoPayload(servoData);
        const response = await fetch('/api/servos', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        if (!response.ok) {
            console.error('Send failed, HTTP', response.status);
            return;
        }
        // No alert popup — update status silently
        console.log('Servos sent', payload);
    } catch (e) {
        console.error('Error sending servos', e);
    }
}

// Periodically fetch MPU-9250 data and display in header
async function fetchImu() {
    try {
        const resp = await fetch('/api/imu');
        if (!resp.ok) return;
        const data = await resp.json();
        const s = `A:(${data.accel.x.toFixed(2)},${data.accel.y.toFixed(2)},${data.accel.z.toFixed(2)}) ` +
                  `G:(${data.gyro.x.toFixed(1)},${data.gyro.y.toFixed(1)},${data.gyro.z.toFixed(1)}) ` +
                  `Ang: P${data.angles.pitch.toFixed(1)} R${data.angles.roll.toFixed(1)} Y${data.angles.yaw.toFixed(1)}`;
        const el = document.getElementById('imuStatus');
        if (el) el.textContent = s;
    } catch (e) {
        // ignore
    }
}

function initUI(){
canvas=document.getElementById("ikCanvas");ctx=canvas.getContext("2d");const canvasContainer=document.getElementById("canvasContainer");
const resizeCanvas=()=>{const rect=canvasContainer.getBoundingClientRect();canvas.width=rect.width;canvas.height=rect.height;};resizeCanvas();
window.addEventListener("resize",resizeCanvas);

// Mouse events
canvas.addEventListener("mousedown",(e)=>{const pos=getMousePosMM(e);const d=Math.hypot(pos.x-uiState.targetX,pos.y-uiState.targetY);if(d<15)uiState.isDragging=true;});
canvas.addEventListener("mousemove",(e)=>{if(!uiState.isDragging)return;const pos=getMousePosMM(e);updateTarget(pos);});
canvas.addEventListener("mouseup",()=>{uiState.isDragging=false;});
canvas.addEventListener("mouseleave",()=>{uiState.isDragging=false;});

// Touch events
canvas.addEventListener("touchstart",(e)=>{e.preventDefault();const touch=e.touches[0];const pos=getTouchPosMM(touch);
const d=Math.hypot(pos.x-uiState.targetX,pos.y-uiState.targetY);if(d<15)uiState.isDragging=true;},false);
canvas.addEventListener("touchmove",(e)=>{e.preventDefault();if(!uiState.isDragging)return;const touch=e.touches[0];const pos=getTouchPosMM(touch);updateTarget(pos);},false);
canvas.addEventListener("touchend",(e)=>{e.preventDefault();uiState.isDragging=false;},false);

document.getElementById("legSelect").addEventListener("change",(e)=>{uiState.selectedLeg=e.target.value;updateConfigPanel();});
document.getElementById("elbowUpToggle").addEventListener("change",(e)=>{uiState.elbowUp=e.target.checked;updatePanelValues();});
document.getElementById("sendNeutralBtn").addEventListener("click",()=>{sendServos({leg:uiState.selectedLeg,angles:{hipDeg:90,kneeDeg:90}});});
document.getElementById("sendCurrentBtn").addEventListener("click",()=>{const hip=parseFloat(document.getElementById("hipOutputValue").textContent);
const knee=parseFloat(document.getElementById("kneeOutputValue").textContent);sendServos({leg:uiState.selectedLeg,angles:{hipDeg:hip,kneeDeg:knee}});});

["hipOffsetInput","hipInvertCheck","hipMinInput","hipMaxInput","hipMinPulseInput","hipMaxPulseInput","kneeOffsetInput","kneeInvertCheck","kneeMinInput","kneeMaxInput","kneeMinPulseInput","kneeMaxPulseInput"].forEach(id=>{
const el = document.getElementById(id); if(!el) return; el.addEventListener("change",()=>{updatePanelValues();autoSendServos();});});

    updateConfigPanel();startAnimation();
    // Start IMU polling
    fetchImu();
    setInterval(fetchImu, 500);
    console.log("✓ UI initialized with animation loop and IMU polling");
}

document.addEventListener("DOMContentLoaded",()=>{console.log("🚀 Démarrage IK...");initUI();console.log("✓ Prêt!");});
</script>
</body>
</html>
)rawliteral";
