#ifndef _ESP32_HOME_PAGE_H
#define _ESP32_HOME_PAGE_H

const char *updatehtml = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 NTP Server</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
            color: #fff;
            min-height: 100vh;
            padding: 20px;
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
        }

        .header {
            text-align: center;
            margin-bottom: 30px;
            padding: 20px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.2);
        }

        .header h1 {
            font-size: 2.5rem;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.3);
        }

        .header p {
            opacity: 0.9;
            font-size: 1.1rem;
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }

        .card {
            background: rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            padding: 25px;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.2);
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }

        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
        }

        .card h2 {
            font-size: 1.4rem;
            margin-bottom: 15px;
            color: #fff;
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .card-icon {
            width: 24px;
            height: 24px;
            opacity: 0.8;
        }

        .info-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }

        .info-item:last-child {
            border-bottom: none;
        }

        .info-label {
            font-weight: 500;
            opacity: 0.9;
        }

        .info-value {
            font-weight: 600;
            text-align: right;
        }

        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-left: 8px;
        }

        .status-online {
            background: #4CAF50;
            box-shadow: 0 0 10px #4CAF50;
        }

        .status-offline {
            background: #f44336;
            box-shadow: 0 0 10px #f44336;
        }

        .btn {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            border: none;
            border-radius: 8px;
            color: white;
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            padding: 12px 20px;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            margin: 5px;
            min-width: 120px;
        }

        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.3);
        }

        .btn:active {
            transform: translateY(0);
        }

        .btn-danger {
            background: linear-gradient(135deg, #ff6b6b 0%, #ee5a24 100%);
        }

        .btn-warning {
            background: linear-gradient(135deg, #feca57 0%, #ff9ff3 100%);
        }

        .btn-success {
            background: linear-gradient(135deg, #48dbfb 0%, #0abde3 100%);
        }

        .toggle-container {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            justify-content: center;
        }

        .control-buttons {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            justify-content: center;
            margin-top: 15px;
        }

        .refresh-btn {
            position: fixed;
            bottom: 30px;
            right: 30px;
            width: 60px;
            height: 60px;
            border-radius: 50%;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            border: none;
            color: white;
            font-size: 24px;
            cursor: pointer;
            box-shadow: 0 5px 20px rgba(0, 0, 0, 0.3);
            transition: all 0.3s ease;
        }

        .refresh-btn:hover {
            transform: scale(1.1);
        }

        .file-upload {
            margin: 15px 0;
        }

        .file-input {
            display: none;
        }

        .file-label {
            background: rgba(255, 255, 255, 0.2);
            border: 2px dashed rgba(255, 255, 255, 0.5);
            border-radius: 8px;
            padding: 20px;
            text-align: center;
            cursor: pointer;
            transition: all 0.3s ease;
            display: block;
        }

        .file-label:hover {
            background: rgba(255, 255, 255, 0.3);
            border-color: rgba(255, 255, 255, 0.8);
        }

        .form-group {
            margin: 15px 0;
        }

        .form-label {
            display: block;
            margin-bottom: 5px;
            font-weight: 500;
            opacity: 0.9;
        }

        .form-input {
            width: 100%;
            padding: 12px 15px;
            border: 2px solid rgba(255, 255, 255, 0.3);
            border-radius: 8px;
            background: rgba(255, 255, 255, 0.1);
            color: white;
            font-size: 14px;
            transition: all 0.3s ease;
        }

        .form-input:focus {
            outline: none;
            border-color: rgba(255, 255, 255, 0.6);
            background: rgba(255, 255, 255, 0.15);
        }

        .form-input::placeholder {
            color: rgba(255, 255, 255, 0.6);
        }

        .progress-bar {
            width: 100%;
            height: 8px;
            background: rgba(255, 255, 255, 0.2);
            border-radius: 4px;
            overflow: hidden;
            margin: 10px 0;
        }

        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #4CAF50, #45a049);
            width: 0%;
            transition: width 0.3s ease;
        }

        @media (max-width: 768px) {
            .grid {
                grid-template-columns: 1fr;
            }
            
            .header h1 {
                font-size: 2rem;
            }
            
            .refresh-btn {
                bottom: 20px;
                right: 20px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🛰️ ESP32 NTP Server</h1>
            <p>Network Time Protocol Server Management Interface</p>
        </div>

        <div class="grid">
            <!-- System Information -->
            <div class="card">
                <h2>
                    <svg class="card-icon" viewBox="0 0 24 24" fill="currentColor">
                        <path d="M12,15.5A3.5,3.5 0 0,1 8.5,12A3.5,3.5 0 0,1 12,8.5A3.5,3.5 0 0,1 15.5,12A3.5,3.5 0 0,1 12,15.5M19.43,12.97C19.47,12.65 19.5,12.33 19.5,12C19.5,11.67 19.47,11.34 19.43,11.03L21.54,9.37C21.73,9.22 21.78,8.95 21.66,8.73L19.66,5.27C19.54,5.05 19.27,4.96 19.05,5.05L16.56,6.05C16.04,5.66 15.5,5.32 14.87,5.07L14.5,2.42C14.46,2.18 14.25,2 14,2H10C9.75,2 9.54,2.18 9.5,2.42L9.13,5.07C8.5,5.32 7.96,5.66 7.44,6.05L4.95,5.05C4.73,4.96 4.46,5.05 4.34,5.27L2.34,8.73C2.22,8.95 2.27,9.22 2.46,9.37L4.57,11.03C4.53,11.34 4.5,11.67 4.5,12C4.5,12.33 4.53,12.65 4.57,12.97L2.46,14.63C2.27,14.78 2.22,15.05 2.34,15.27L4.34,18.73C4.46,18.95 4.73,19.03 4.95,18.95L7.44,17.94C7.96,18.34 8.5,18.68 9.13,18.93L9.5,21.58C9.54,21.82 9.75,22 10,22H14C14.25,22 14.46,21.82 14.5,21.58L14.87,18.93C15.5,18.68 16.04,18.34 16.56,17.94L19.05,18.95C19.27,19.03 19.54,18.95 19.66,18.73L21.66,15.27C21.78,15.05 21.73,14.78 21.54,14.63L19.43,12.97Z"/>
                    </svg>
                    System Information
                </h2>
                <div class="info-item">
                    <span class="info-label">ESP32 Board Build</span>
                    <span class="info-value" id="firmware-board">none</span>
                </div>
                <div class="info-item">
                    <span class="info-label">Firmware Version</span>
                    <span class="info-value" id="firmware-version">v2.1.3</span>
                </div>
                <div class="info-item">
                    <span class="info-label">Free Heap Memory</span>
                    <span class="info-value" id="free-heap">245.2 KB</span>
                </div>
                <div class="info-item">
                    <span class="info-label">System Uptime</span>
                    <span class="info-value" id="uptime">2d 14h 32m</span>
                </div>
                <div class="info-item">
                    <span class="info-label">MCU Temperature</span>
                    <span class="info-value" id="cpu-temp">42°C</span>
                </div>
            </div>

            <!-- GPS Information -->
            <div class="card">
                <h2>
                    <svg class="card-icon" viewBox="0 0 24 24" fill="currentColor">
                        <path d="M12,8A4,4 0 0,1 16,12A4,4 0 0,1 12,16A4,4 0 0,1 8,12A4,4 0 0,1 12,8M3.05,13H1V11H3.05C3.5,6.83 6.83,3.5 11,3.05V1H13V3.05C17.17,3.5 20.5,6.83 20.95,11H23V13H20.95C20.5,17.17 17.17,20.5 13,20.95V23H11V20.95C6.83,20.5 3.5,17.17 3.05,13M12,5A7,7 0 0,0 5,12A7,7 0 0,0 12,19A7,7 0 0,0 19,12A7,7 0 0,0 12,5Z"/>
                    </svg>
                    GPS Status
                </h2>
                <div class="info-item">
                    <span class="info-label">Satellites in View</span>
                    <span class="info-value" id="satellites-view">0</span>
                </div>
                <div class="info-item">
                    <span class="info-label">GPS Lock Status</span>
                    <span class="info-value" id="gps-lock">
                        No<span class="status-indicator status-online"></span>
                    </span>
                </div>
                <div class="info-item">
                    <span class="info-label">Date / Time</span>
                    <span class="info-value" id="gps-date">No Data</span>
                </div>
                <div class="info-item">
                    <span class="info-label">NTP Server Status</span>
                    <span class="info-value" id="ntp-status">
                        Running<span class="status-indicator status-online"></span>
                    </span>
                </div>
                <div class="info-item">
                    <span class="info-label">Requests/Hour</span>
                    <span class="info-value" id="ntp-requests">0</span>
                </div>
            </div>

            <!-- Display Controls -->
            <div class="card">
                <h2>
                    <svg class="card-icon" viewBox="0 0 24 24" fill="currentColor">
                        <path d="M21,16.5C21,16.88 20.79,17.21 20.47,17.38L12.57,21.82C12.41,21.94 12.21,22 12,22C11.79,22 11.59,21.94 11.43,21.82L3.53,17.38C3.21,17.21 3,16.88 3,16.5V7.5C3,7.12 3.21,6.79 3.53,6.62L11.43,2.18C11.59,2.06 11.79,2 12,2C12.21,2 12.41,2.06 12.57,2.18L20.47,6.62C20.79,6.79 21,7.12 21,7.5V16.5M12,4.15L5,8.09V15.91L12,19.85L19,15.91V8.09L12,4.15Z"/>
                    </svg>
                    Display Controls
                </h2>
                <div class="info-item">
                    <span class="info-label">Main Display</span>
                    <span class="info-value" id="main-display-status">
                        ON<span class="status-indicator status-online"></span>
                    </span>
                </div>
                <div class="toggle-container">
                    <button class="btn btn-success" onclick="toggleDisplay() ">Toggle Display</button>
                </div>
            </div>

            <!-- System Controls -->
            <div class="card">
                <h2>
                    <svg class="card-icon" viewBox="0 0 24 24" fill="currentColor">
                        <path d="M12,16A3,3 0 0,1 9,13C9,11.88 9.61,10.9 10.5,10.39L20.21,4.77L14.68,14.35C14.18,15.33 13.17,16 12,16M12,3C13,3 14,3.25 14.97,3.68L12,9L9.03,3.68C10,3.25 11,3 12,3M12,21C11,21 10,20.75 9.03,20.32L12,15L14.97,20.32C14,20.75 13,21 12,21M21,12C21,13 20.75,14 20.32,14.97L15,12L20.32,9.03C20.75,10 21,11 21,12M3,12C3,11 3.25,10 3.68,9.03L9,12L3.68,14.97C3.25,14 3,13 3,12Z"/>
                    </svg>
                    System Controls
                </h2>
                <div class="info-item">
                    <span class="info-label">Hostname</span>
                    <span class="info-value" id="hostname-id">none</span>
                </div>
                <div class="control-buttons">
                    <button class="btn btn-warning" onclick="rebootSystem() ">🔄 Reboot System</button>
                    <button class="btn btn-danger" onclick="resetSystem() ">⚠️ Factory Reset</button>
                </div>
            </div>

            <!-- Admin Configuration -->
            <div class="card">
                <h2>
                    <svg class="card-icon" viewBox="0 0 24 24" fill="currentColor">
                        <path d="M12,1L3,5V11C3,16.55 6.84,21.74 12,23C17.16,21.74 21,16.55 21,11V5L12,1M12,7C13.4,7 14.8,8.6 14.8,10V11.4C15.2,11.7 15.5,12.2 15.5,12.8V16.3C15.5,17.2 14.8,17.8 13.9,17.8H10.1C9.2,17.8 8.5,17.1 8.5,16.2V12.8C8.5,12.1 8.8,11.6 9.2,11.3V10C9.2,8.6 10.6,7 12,7M12,8.2C11.2,8.2 10.5,8.7 10.5,9.5V11.3H13.5V9.5C13.5,8.7 12.8,8.2 12,8.2Z"/>
                    </svg>
                    Change Admin Data
                </h2>
                <div class="form-group">
                    <label class="form-label" for="admin-id">Admin ID:</label>
                    <input type="text" id="admin-id" class="form-input" placeholder="Enter admin username" maxlength="32">
                </div>
                <div class="form-group">
                    <label class="form-label" for="admin-password">Admin Password:</label>
                    <input type="password" id="admin-password" class="form-input" placeholder="Enter admin password" maxlength="64">
                </div>
                <div class="form-group">
                    <label class="form-label" for="hostname">Device Hostname:</label>
                    <input type="text" id="hostname" class="form-input" placeholder="Enter device hostname" maxlength="32">
                </div>
                <div class="control-buttons">
                    <button class="btn btn-success" onclick="updateAdminSettings() ">💾 Update Settings</button>
                </div>
            </div>

            <!-- Firmware Update -->
            <div class="card">
                <h2>
                    <svg class="card-icon" viewBox="0 0 24 24" fill="currentColor">
                        <path d="M14,2H6A2,2 0 0,0 4,4V20A2,2 0 0,0 6,22H18A2,2 0 0,0 20,20V8L14,2M18,20H6V4H13V9H18V20Z"/>
                    </svg>
                    Firmware Update
                </h2>
                <div class="file-upload">
                    <label for="firmware-file" class="file-label">
                        📁 Click to select firmware file (.bin)
                        <div style="font-size: 0.9em; opacity: 0.8; margin-top: 5px;">
                            Max file size: 2MB
                        </div>
                    </label>
                    <input type="file" id="firmware-file" class="file-input" accept=".bin" onchange="selectFirmware(this) ">
                </div>
                <div id="upload-progress" class="progress-bar" style="display: none;">
                    <div class="progress-fill" id="progress-fill"></div>
                </div>
                <div class="control-buttons" style="margin-top: 15px;">
                    <button class="btn btn-success" id="upload-btn" onclick="uploadFirmware() " disabled>📤 Upload Firmware</button>
                </div>
            </div>
        </div>
    </div>

    <button class="refresh-btn" onclick="refreshData() " title="Refresh Data">🔄</button>

    <script>
        let selectedFile = null;
        let refreshInterval;

        // Start auto-refresh
        function startAutoRefresh() {
            refreshInterval = setInterval(refreshData, 10000); // Refresh every 10 seconds
        }

        // Stop auto-refresh
        function stopAutoRefresh() {
            if (refreshInterval) {
                clearInterval(refreshInterval);
            }
        }

        // Refresh system data
        async function refreshData() {
            try {
                const response = await fetch('/api/status');
                if (response.ok) {
                    const data = await response.json();
                    updateUI(data);
                }
            } catch (error) {
                console.error('Failed to refresh data:', error);
            }
        }

        // Update UI with fresh data
        function updateUI(data) {
            // System information
            document.getElementById('firmware-version').textContent = data.firmware || 'v0';
            document.getElementById('firmware-board').textContent = data.board || 'none';
            document.getElementById('hostname-id').textContent = data.hostname || 'none';
            document.getElementById('free-heap').textContent = data.freeHeap || '0 KB';
            document.getElementById('uptime').textContent = data.uptime || '0s';
            document.getElementById('cpu-temp').textContent = data.temperature || '0°C';

            // GPS information
            document.getElementById('satellites-view').textContent = data.gps?.satellitesInView || '0';
            document.getElementById('gps-date').textContent = data.gps?.datetime || 'NO JSON';

            // Update GPS lock status
            const gpsLockElement = document.getElementById('gps-lock');
            const gpsStatus = data.gps?.lockStatus || 'No Fix';
            const gpsIndicator = data.gps?.hasLock ? 'status-online' : 'status-offline';
            gpsLockElement.innerHTML = `${gpsStatus}<span class="status-indicator ${gpsIndicator}"></span>`;

            // Display status
            updateDisplayStatus('main-display-status', data.display?.main);

            // NTP status
            document.getElementById('ntp-requests').textContent = data.ntp?.requestsPerHour || '0';

            const ntpStatusElement = document.getElementById('ntp-status');
            const ntpRunning = data.ntp?.running !== false;
            const ntpIndicator = ntpRunning ? 'status-online' : 'status-offline';
            ntpStatusElement.innerHTML = `${ntpRunning ? 'Running' : 'Stopped'}<span class="status-indicator ${ntpIndicator}"></span>`;
        }

        // Update display status indicators
        function updateDisplayStatus(elementId, status) {
            const element = document.getElementById(elementId);
            const isOn = status !== false;
            const indicator = isOn ? 'status-online' : 'status-offline';
            element.innerHTML = `${isOn ? 'ON' : 'OFF'}<span class="status-indicator ${indicator}"></span>`;
        }

        // Toggle display functions
        async function toggleDisplay() {
            try {
		const response = await fetch('/api/toggle', { method: 'GET' });
                if (response.ok) {
                    refreshData(); // Refresh to get updated status
                }
            } catch (error) {
                console.error('Failed to toggle:', error);
            }
        }

        // System control functions
        async function rebootSystem() {
            if (confirm('Are you sure you want to reboot the system?')) {
                try {
                    await fetch('/api/reboot', { method: 'GET' });
                    alert('System is rebooting...');
                    stopAutoRefresh();
                    setTimeout(() => {
                        window.location.href = '/'; 
                    }, 3000);
                } catch (error) {
                    console.error('Failed to reboot:', error);
                }
            }
        }

        async function resetSystem() {
            if (confirm('Are you sure you want to perform a factory reset? This will erase all settings!')) {
                if (confirm('This action cannot be undone. Are you absolutely sure?')) {
                    try {
                        await fetch('/api/reset', { method: 'GET' });
                        alert('Factory reset initiated...');
                        alert('Remember to connect to NTPS1 wifi and go to http://192.168.5.1');
                        stopAutoRefresh();
                        setTimeout(() => {
                            window.location.href = 'http://192.168.5.1/'; 
                        }, 15000);
                    } catch (error) {
                        console.error('Failed to reset:', error);
                    }
                }
            }
        }

        // Firmware update functions
        function selectFirmware(input) {
            const file = input.files[0];
            if (file) {
                if (file.size > 1310700 ) { // limit
                    alert('File too large. Maximum size is 2MB.');
                    input.value = '';
                    return;
                }
                selectedFile = file;
                document.getElementById('upload-btn').disabled = false;
                document.querySelector('.file-label').innerHTML = `
                    📁 Selected: ${file.name}
                    <div style="font-size: 0.9em; opacity: 0.8; margin-top: 5px;">
                        Size: ${(file.size / 1024).toFixed(1)} KB
                    </div>
                `;
            }
        }

        async function uploadFirmware() {
            if (!selectedFile) return;

            if (!confirm('Are you sure you want to update the firmware? The device will reboot after upload.')) {
                return;
            }

            const progressBar = document.getElementById('upload-progress');
            const progressFill = document.getElementById('progress-fill');
            const uploadBtn = document.getElementById('upload-btn');

            progressBar.style.display = 'block';
            uploadBtn.disabled = true;
            uploadBtn.textContent = 'Uploading...';

            const formData = new FormData();
            formData.append('firmware', selectedFile);

            try {
                const xhr = new XMLHttpRequest();
                
                xhr.upload.addEventListener('progress', (e) => {
                    if (e.lengthComputable) {
                        const percentComplete = (e.loaded / e.total) * 100;
                        progressFill.style.width = percentComplete + '%';
                    }
                });

                xhr.addEventListener('load', () => {
                    if (xhr.status === 200) {
                        progressFill.style.width = '100%';
                        alert('Firmware uploaded successfully! Device is rebooting...');
                        stopAutoRefresh();
                        setTimeout(() => {
                            window.location.href = '/'; 
                        }, 3000);
                    } else {
                        throw new Error('Upload failed');
                    }
                });

                xhr.addEventListener('error', () => {
                    throw new Error('Upload failed');
                });

                xhr.open('POST', '/api/firmware');
                xhr.send(formData);

            } catch (error) {
                console.error('Upload failed:', error);
                alert('Firmware upload failed. Please try again.');
                progressBar.style.display = 'none';
                uploadBtn.disabled = false;
                uploadBtn.textContent = '📤 Upload Firmware';
            }
        }

        // Update admin settings
        async function updateAdminSettings() {
            const adminId = document.getElementById('admin-id').value.trim();
            const adminPassword = document.getElementById('admin-password').value.trim();
            const hostname = document.getElementById('hostname').value.trim();

            if (!adminId && !adminPassword && !hostname) {
                alert('Please fill in on field');
                return;
            }

            if ( adminId && adminId.length < 3 || adminId.length > 32) {
                alert('Admin ID must be between 3 and 32 characters');
                return;
            }

            if (adminPassword && adminPassword.length < 6 || adminPassword.length > 64) {
                alert('Admin password must be between 6 and 64 characters');
                return;
            }

            if (hostname && hostname.length < 3 || hostname.length > 32) {
                alert('Hostname must be between 3 and 32 characters');
                return;
            }

            if ( hostname && !/^[a-zA-Z0-9-]+$/.test(hostname)) {
                alert('Hostname can only contain letters, numbers, and hyphens');
                return;
            }

            if (!confirm('Are you sure you want to update the admin settings? You may need to re-login.')) {
                return;
            }

            try {
                const response = await fetch('/api/admin-config', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({
                        adminId: adminId,
                        adminPassword: adminPassword,
                        hostname: hostname
                    })
                });

                if (response.ok) {
                    alert('Admin settings updated successfully!');
                    document.getElementById('admin-id').value = '';
                    document.getElementById('admin-password').value = '';
                    document.getElementById('hostname').value = '';
                    setTimeout(() => {
                        window.location.href = '/'; 
                    }, 3000);
                } else {
                    throw new Error('Failed to update settings');
                }
            } catch (error) {
                console.error('Failed to update admin settings:', error);
                alert('Failed to update admin settings. Please try again.');
            }
        }

        // Initialize the interface
        document.addEventListener('DOMContentLoaded', function() {
            refreshData(); // Initial data load
            startAutoRefresh(); // Start periodic refresh
        });

        // Handle visibility change to pause/resume refresh
        document.addEventListener('visibilitychange', function() {
            if (document.hidden) {
                stopAutoRefresh();
            } else {
                startAutoRefresh();
            }
        });
    </script>
</body>
</html>
)";

#endif
