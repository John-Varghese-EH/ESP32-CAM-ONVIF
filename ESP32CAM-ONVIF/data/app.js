// --- Login Overlay Logic ---
let authHeader = "";
document.addEventListener("DOMContentLoaded", function() {
  if (localStorage.getItem("auth")) {
    authHeader = localStorage.getItem("auth");
    document.getElementById("login-overlay").style.display = "none";
    updateStatus();
  }
});
function login(e) {
  e.preventDefault();
  const user = document.getElementById("username").value;
  const pass = document.getElementById("password").value;
  authHeader = "Basic " + btoa(user + ":" + pass);
  fetch("/api/status", {headers: {"Authorization": authHeader}})
    .then(r => {
      if (r.status === 401) throw new Error("Unauthorized");
      return r.json();
    })
    .then(() => {
      document.getElementById("login-overlay").style.display = "none";
      localStorage.setItem("auth", authHeader);
      updateStatus();
    })
    .catch(() => {
      document.getElementById("login-error").textContent = "Invalid credentials!";
      authHeader = "";
      localStorage.removeItem("auth");
    });
  return false;
}

// --- API Helpers ---
function api(url, opts={}) {
  opts.headers = opts.headers || {};
  opts.headers["Authorization"] = authHeader;
  return fetch(url, opts);
}

// --- Tab Navigation ---
function showPanel(panel) {
  document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
  document.querySelectorAll('.panel').forEach(sec => sec.style.display = 'none');
  document.querySelector('.tab-btn[onclick="showPanel(\''+panel+'\')"]').classList.add('active');
  document.getElementById('panel-' + panel).style.display = '';
  if (panel === 'sd') refreshSD();
}

// --- SD Card Management ---
function refreshSD() {
  api('/api/sd/list').then(r => r.json()).then(files => {
    const tbody = document.querySelector('#sd-table tbody');
    tbody.innerHTML = '';
    files.forEach(file => {
      const tr = document.createElement('tr');
      tr.innerHTML = `<td>${file.name}</td>
        <td>${(file.size/1024).toFixed(1)} KB</td>
        <td>
          <a href="/api/sd/download?file=${encodeURIComponent(file.name)}" class="btn" target="_blank">Download</a>
          <button class="btn" onclick="deleteSD('${file.name}')">Delete</button>
        </td>`;
      tbody.appendChild(tr);
    });
  });
}
function deleteSD(filename) {
  if (!confirm('Delete ' + filename + '?')) return;
  api('/api/sd/delete', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({file: filename})
  }).then(refreshSD);
}

// --- UI Logic ---
function updateStatus() {
  api('/api/status').then(r => r.json()).then(data => {
    document.getElementById('status').textContent = data.status || 'Online';
    document.getElementById('rtspUrl').textContent = data.rtsp || '';
    document.getElementById('onvifUrl').textContent = data.onvif || '';
    document.getElementById('motionStatus').textContent = "Motion: " + (data.motion ? "Detected" : "None");
  }).catch(() => {
    document.getElementById('status').textContent = 'Offline';
    if (!document.getElementById("login-overlay").style.display || document.getElementById("login-overlay").style.display === "none") {
      document.getElementById("login-overlay").style.display = "flex";
    }
    localStorage.removeItem("auth");
  });
}
function setConfig(key, value) {
  api('/api/config', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({[key]: value})
  }).then(updateStatus);
}
function snapshot() {
  window.open('/snapshot', '_blank');
}
function toggleRecording() {
  api('/api/record', {method: 'POST'}).then(updateStatus);
}
function reboot() {
  api('/api/reboot', {method: 'POST'});
  alert("Rebooting...");
}
function factoryReset() {
  api('/api/factory_reset', {method: 'POST'});
  alert("Factory reset initiated.");
}
setInterval(updateStatus, 2000);
updateStatus();
