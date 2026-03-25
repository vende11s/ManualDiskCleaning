const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const { spawn } = require('child_process');
const fs = require('fs');

let backendProcess = null;

function startBackend() {
  const backendPath = path.join(__dirname, 'getSpace-backend.exe');
  if (fs.existsSync(backendPath)) {
    try {
      backendProcess = spawn(backendPath, [], {
        cwd: __dirname,
        stdio: 'ignore'
      });

      backendProcess.on('error', (err) => console.error('Backend spawn error:', err));
      backendProcess.on('exit', (code) => console.log('Backend died, code:', code));

    } catch (e) {
      console.error('Exception starting backend:', e);
    }
  }
}

function createWindow() {
  const win = new BrowserWindow({
    width: 1000,
    height: 650,
    icon: path.join(__dirname, 'resources/logo.ico'),
    frame: false,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true
    }
  });

  win.loadFile('select.html');
}

app.whenReady().then(() => {
  startBackend();
  createWindow();

  ipcMain.on('window-minimize', (event) => {
    const win = BrowserWindow.fromWebContents(event.sender);
    if (win) win.minimize();
  });

  ipcMain.on('window-maximize', (event) => {
    const win = BrowserWindow.fromWebContents(event.sender);
    if (win) {
      if (win.isMaximized()) {
        win.unmaximize();
      } else {
        win.maximize();
      }
    }
  });

  ipcMain.on('window-close', (event) => {
    const win = BrowserWindow.fromWebContents(event.sender);
    if (win) win.close();
  });

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow();
  });
});

app.on('will-quit', () => {
  if (backendProcess) {
    try {
      backendProcess.kill();
    } catch (e) {
      console.error('Error killing backend:', e);
    }
  }
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});