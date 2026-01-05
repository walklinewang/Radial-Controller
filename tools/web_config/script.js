class SerialAssistant {
    constructor() {
        // 命令常量
        this.COMMANDS = {
            CONFIG_MODE_ENABLE: 'config_mode_enabled',
            CONFIG_MODE_DISABLE: 'config_mode_disabled',
            GET_CONFIG: 'get_config',
            SHOW_MENU: 'show_menu',
            RESET_CONFIG: 'reset_config',
            SET_PREFIX: 'set_'
        };

        // 响应常量
        this.RESPONSES = {
            CONFIG_MODE_ENABLED_SUCCESS: 'config_mode_enabled_success'
        };

        this.port = null;
        this.reader = null;
        this.writer = null;
        this.isConnected = false;
        this.connectionCheckInterval = null;
        this.usbVendorId = 0x1209;
        this.receiveBuffer = '';
        this.dataReceivedResolver = null;

        // 配置参数
        this.configParams = {
            led_count: { label: 'LED灯珠数量', type: 'number', min: 1, max: 255, value: 12 },
            color_order: { label: 'LED颜色顺序', type: 'select', options: [{ value: 0, label: 'GRB' }, { value: 1, label: 'RGB' }], value: 0 },
            brightness: { label: '亮度等级', type: 'number', min: 0, max: 4, value: 2 },
            effect_mode: { label: 'LED灯效模式', type: 'number', min: 0, max: 255, value: 0 },
            effect_tick: { label: 'LED灯效循环周期(ms)', type: 'number', min: 100, max: 10000, value: 1000 },
            rotate_cw: { label: '顺时针旋转角度', type: 'number', min: -360, max: 360, value: 0 },
            rotate_ccw: { label: '逆时针旋转角度', type: 'number', min: -360, max: 360, value: 0 }
        };

        this.initElements();
        this.initEventListeners();
        this.checkBrowserSupport();
        this.generateConfigControls();
        this.updateUIState();
    }

    clearTimer() {
        if (this.connectionCheckInterval) {
            clearInterval(this.connectionCheckInterval);
            this.connectionCheckInterval = null;
        }
        if (this.waitForDataTimer) {
            clearTimeout(this.waitForDataTimer);
            this.waitForDataTimer = null;
        }
    }

    isDeviceLost(error) {
        return error.message.includes('device has been lost') ||
            error.name === 'NetworkError' ||
            error.name === 'AbortError';
    }

    releaseResource(resource) {
        if (resource) {
            try {
                if (typeof resource.releaseLock === 'function') {
                    resource.releaseLock();
                } else if (typeof resource.close === 'function') {
                    resource.close();
                }
            } catch (error) {
                console.warn('释放资源失败:', error.message);
            }
        }
        return null;
    }

    addLineEnding(data, endingType) {
        const endings = { 'lf': '\n', 'cr': '\r', 'crlf': '\r\n', 'none': '' };
        const targetEnding = endings[endingType] || '';

        if (!data || !targetEnding) {
            return data + targetEnding;
        }

        const hasCRLF = data.includes('\r\n');
        const hasLF = data.includes('\n') && !hasCRLF;
        const hasCR = data.includes('\r') && !hasCRLF;

        let processedData = data + (data.endsWith(targetEnding) ? '' : targetEnding);

        if (hasCRLF && targetEnding !== '\r\n') {
            processedData = processedData.replace(/\r\n/g, targetEnding);
        } else if (hasLF && targetEnding !== '\n') {
            processedData = processedData.replace(/\n/g, targetEnding);
        } else if (hasCR && targetEnding !== '\r') {
            processedData = processedData.replace(/\r/g, targetEnding);
        }

        return processedData;
    }

    getSerialConfig() {
        // 使用默认配置，因为连接参数设置区域已移除
        return {
            baudRate: 115200,
            dataBits: 8,
            stopBits: 1,
            parity: 'none'
        };
    }

    handleConnectionError(error) {
        const errorMap = {
            'NotOpenError': { title: '串口被占用', content: '该串口已被其他应用占用，请关闭其他应用后重试' },
            'PermissionDeniedError': { title: '权限被拒绝', content: '无权限访问该串口' },
            'NotFoundError': { title: '未找到串口', content: '未找到所选串口' }
        };

        const errorInfo = errorMap[error.name] ||
            (error.message.includes('occupied') || error.message.includes('in use') ?
                { title: '串口被占用', content: '该串口已被其他应用占用，请关闭其他应用后重试' } : null);

        if (errorInfo) {
            this.showCustomAlert(errorInfo.title, errorInfo.content);
            return `串口连接失败: ${errorInfo.content}`;
        }

        this.showCustomAlert('连接失败', error.message);
        return `连接失败: ${error.message}`;
    }

    handleConnectionLost() {
        if (this.isConnected) {
            this.isConnected = false;
            this.clearTimer();
            this.updateUIState();
            this.showStatus('串口连接已断开 (设备可能已复位)', 'error');
            this.showCustomAlert('连接断开', '串口连接已断开，设备可能已复位，请重新连接');
        }
    }

    async getWriter() {
        if (!this.port || !this.port.writable) {
            throw new Error('串口已断开连接');
        }
        if (!this.writer) {
            this.writer = this.port.writable.getWriter();
        }
        return this.writer;
    }

    initElements() {
        this.browserSupportAlert = document.getElementById('browser-support-alert');
        this.browserInfo = document.getElementById('browser-info');
        this.connectToggleBtn = document.getElementById('connect-toggle-btn');
        this.connectionStatusDot = document.getElementById('connection-status-dot');
        this.connectionStatusText = document.getElementById('connection-status-text');
        this.configContainer = document.getElementById('config-container');
        this.firmwareVersion = document.getElementById('firmware-version');
        this.toolVersion = document.getElementById('tool-version');
        this.resetSettingsBtn = document.getElementById('reset-settings-btn');
        this.reloadSettingsBtn = document.getElementById('reload-settings-btn');
        this.saveSettingsBtn = document.getElementById('save-settings-btn');
        this.connectionOverlay = document.getElementById('connection-overlay');
        this.customAlert = document.getElementById('custom-alert');
        this.alertTitle = document.getElementById('alert-title');
        this.alertMessage = document.getElementById('alert-message');
        this.alertOkBtn = document.getElementById('alert-ok');
    }

    initEventListeners() {
        this.connectToggleBtn.addEventListener('click', () => this.isConnected ? this.disconnect() : this.connect());
        this.resetSettingsBtn.addEventListener('click', () => this.resetSettings());
        this.reloadSettingsBtn.addEventListener('click', () => this.reloadSettings());
        this.saveSettingsBtn.addEventListener('click', () => this.saveSettings());

        this.alertOkBtn.addEventListener('click', () => this.hideCustomAlert());
    }

    generateConfigControls() {
        if (!this.configContainer) return;

        this.configContainer.innerHTML = '';

        Object.keys(this.configParams).forEach(paramKey => {
            const param = this.configParams[paramKey];
            const container = document.createElement('div');
            container.className = 'config-item';

            const label = document.createElement('label');
            label.textContent = param.label;
            label.setAttribute('for', `config-${paramKey}`);
            container.appendChild(label);

            let input;
            if (param.type === 'select') {
                input = document.createElement('select');
                input.id = `config-${paramKey}`;
                input.name = paramKey;

                param.options.forEach(option => {
                    const opt = document.createElement('option');
                    opt.value = option.value;
                    opt.textContent = option.label;
                    if (option.value === param.value) {
                        opt.selected = true;
                    }
                    input.appendChild(opt);
                });
            } else {
                input = document.createElement('input');
                input.type = param.type;
                input.id = `config-${paramKey}`;
                input.name = paramKey;
                input.value = param.value;
                input.min = param.min;
                input.max = param.max;
                if (param.type === 'number') {
                    input.step = '1';
                }
            }

            input.addEventListener('input', (e) => {
                this.configParams[paramKey].value = param.type === 'number' ? parseInt(e.target.value) : e.target.value;
            });

            container.appendChild(input);
            this.configContainer.appendChild(container);
        });
    }

    async connect() {
        try {
            if (!this.checkBrowserSupport()) return;

            await this.cleanupResources();
            this.showStatus('正在连接串口...', 'success');

            const config = this.getSerialConfig();
            console.log('串口配置:', config);

            let port = await navigator.serial.requestPort({ filters: [{ usbVendorId: this.usbVendorId }] });

            await port.open(config);
            this.port = port;
            
            // this.showStatus(`串口连接成功 (${config.baudRate} bps, ${config.dataBits}N${config.stopBits}, ${config.parity})`, 'success');

            // 开始接收数据
            this.startReading();

            // 连接成功后发送命令，将is_config_mode设置为true
            await this.enableConfigMode();

            this.updateUIState();

            this.connectionCheckInterval = setInterval(() => this.checkConnectionStatus(), 5000);
        } catch (error) {
            if (error.name === 'NotFoundError') {
                this.showStatus('未选择串口，请重新连接并选择有效串口', 'warning');
                return;
            }
            const errorMessage = this.handleConnectionError(error);
            this.showStatus(errorMessage, 'error');
            console.error('连接串口错误:', error);
        }
    }
    
    /**
     * 开始接收串口数据
     */
    async startReading() {
        if (!this.port || !this.port.readable) {
            return;
        }
        
        try {
            const decoder = new TextDecoderStream();
            this.reader = this.port.readable.pipeThrough(decoder).getReader();
            
            while (true) {
                const { value, done } = await this.reader.read();
                if (done) break;
                
                if (value) {
                    this.receiveBuffer += value;
                    this.processReceivedData();
                }
            }
        } catch (error) {
            console.error('接收数据错误:', error);
            if (!this.isConnected) return;
            this.showStatus('接收数据异常，已断开连接', 'error');
            await this.disconnect();
        } finally {
            this.reader = this.releaseResource(this.reader);
        }
    }
    
    /**
     * 处理接收到的数据
     */
    processReceivedData() {
        // 分割接收到的数据，按行处理
        const lines = this.receiveBuffer.split('\n');
        this.receiveBuffer = lines.pop(); // 保留最后一行（可能不完整）
        
        for (const line of lines) {
            // 去掉回车符和空白字符
            const trimmedLine = line.replace(/\r/g, '').trim();
            if (!trimmedLine) continue;
            
            // 检查是否是配置数据
            if (trimmedLine.includes('=')) {
                const [key, value] = trimmedLine.split('=');
                const paramKey = key.trim();
                const paramValue = value.trim();
                
                // 更新配置参数
                if (this.configParams[paramKey]) {
                    this.configParams[paramKey].value = paramKey === 'color_order' ? parseInt(paramValue) : parseInt(paramValue);
                    // 更新UI控件
                    this.updateConfigControl(paramKey, parseInt(paramValue));
                }
            }
            
            // 通知等待数据的promise
            if (this.dataReceivedResolver) {
                this.dataReceivedResolver(trimmedLine);
            }
        }
    }
    
    /**
     * 等待接收数据
     * @param {number} timeout - 超时时间（毫秒）
     * @returns {Promise<string>} 接收到的数据
     */
    waitForData(timeout = 1000) {
        return new Promise((resolve, reject) => {
            this.dataReceivedResolver = resolve;
            
            // 设置超时
            const timer = setTimeout(() => {
                this.dataReceivedResolver = null;
                reject(new Error('等待数据超时'));
            }, timeout);
            
            // 保存timer以便清理
            this.waitForDataTimer = timer;
        });
    }

    async disconnect() {
        try {
            this.showStatus('正在断开串口...', 'warning');
            await this.cleanupResources();
            this.showStatus('串口已断开', 'success');
        } catch (error) {
            console.error('断开串口错误:', error);
            this.isConnected = false;
            this.updateUIState();
            this.showStatus(`断开失败: ${error.message}`, 'error');
        }
    }

    async cleanupResources() {
        this.isConnected = false;
        this.updateUIState();
        this.reader = this.releaseResource(this.reader);
        this.writer = this.releaseResource(this.writer);
        this.clearTimer();
        
        // 重置接收缓冲区和数据解析器
        this.receiveBuffer = '';
        this.dataReceivedResolver = null;

        if (this.port) {
            try {
                await Promise.race([
                    this.port.close(),
                    new Promise(resolve => setTimeout(resolve, 500))
                ]);
            } catch (portError) {
                console.warn('关闭端口失败或超时:', portError.message);
            } finally {
                this.port = null;
            }
        }
    }

    checkConnectionStatus() {
        if (!this.isConnected) {
            this.clearTimer();
            return;
        }
        if (!this.port || !this.port.readable) {
            this.handleConnectionLost();
        }
    }

    async enableConfigMode() {
        try {
            const writer = await this.getWriter();
            const command = this.COMMANDS.CONFIG_MODE_ENABLE + '\n';
            const sendBuffer = new TextEncoder().encode(command);
            await writer.write(sendBuffer);
            this.showStatus('正在启用配置模式...', 'info');

            const response = await this.waitForData(1000);

            if (response === this.RESPONSES.CONFIG_MODE_ENABLED_SUCCESS) {
                this.showStatus('配置模式已成功启用', 'success');
                // 显示设置覆盖层
                this.isConnected = true;
            } else {
                throw new Error(`意外响应: ${response}`);
            }
        } catch (error) {
            this.writer = this.releaseResource(this.writer);
            this.showStatus(`启用配置模式失败: ${error.message}`, 'error');
        }
    }

    async resetSettings() {
        if (!this.isConnected) {
            this.showStatus('请先连接串口', 'warning');
            return;
        }

        try {
            this.showStatus('正在重置配置...', 'info');
            const writer = await this.getWriter();
            
            // 发送重置配置命令
            const resetCommand = this.COMMANDS.RESET_CONFIG + '\n';
            const resetBuffer = new TextEncoder().encode(resetCommand);
            await writer.write(resetBuffer);
            
            // 等待设备重置配置
            await new Promise(resolve => setTimeout(resolve, 500));
            
            // 重新加载配置
            await this.reloadSettings();
            
            this.showStatus('配置已重置为默认值', 'success');
        } catch (error) {
            this.showStatus(`重置配置失败: ${error.message}`, 'error');
        }
    }

    /**
     * 更新配置控件的值
     * @param {string} key - 参数键名
     * @param {number} value - 参数值
     */
    updateConfigControl(key, value) {
        const control = document.getElementById(`config-${key}`);
        if (control) {
            control.value = value;
        }
    }
    
    async reloadSettings() {
        if (!this.isConnected) {
            this.showStatus('请先连接串口', 'warning');
            return;
        }

        try {
            this.showStatus('正在重新加载配置...', 'info');
            const writer = await this.getWriter();
            
            // 发送读取配置命令
            const getConfigCommand = this.COMMANDS.GET_CONFIG + '\n';
            const getConfigBuffer = new TextEncoder().encode(getConfigCommand);
            await writer.write(getConfigBuffer);
            
            // 等待设备返回配置信息
            await new Promise(resolve => setTimeout(resolve, 500));
            
            this.showStatus('配置已重新加载', 'success');
        } catch (error) {
            this.showStatus(`重新加载配置失败: ${error.message}`, 'error');
        }
    }

    async saveSettings() {
        if (!this.isConnected) {
            this.showStatus('请先连接串口', 'warning');
            return;
        }

        try {
            const writer = await this.getWriter();

            // 发送每个配置参数
            for (const [key, param] of Object.entries(this.configParams)) {
                const command = `${this.COMMANDS.SET_PREFIX}${key}=${param.value}\n`;
                const sendBuffer = new TextEncoder().encode(command);
                await writer.write(sendBuffer);
                // 等待一小段时间确保命令被正确处理
                await new Promise(resolve => setTimeout(resolve, 10));
            }

            // 发送保存配置命令
            const saveCommand = 'save_config\n';
            const saveBuffer = new TextEncoder().encode(saveCommand);
            await writer.write(saveBuffer);

            this.showStatus('配置已保存到设备', 'success');
        } catch (error) {
            this.writer = this.releaseResource(this.writer);
            this.showStatus(`保存配置失败: ${error.message}`, 'error');
        }
    }

    updateUIState() {
        const isConnected = this.isConnected;
        this.connectToggleBtn.textContent = isConnected ? '断开设备' : '连接设备';
        this.connectToggleBtn.className = isConnected ? 'secondary-btn' : 'primary-btn';

        // 更新连接状态指示灯
        if (this.connectionStatusDot) {
            this.connectionStatusDot.className = isConnected ? 'status-dot connected' : 'status-dot';
        }

        // 更新连接状态文本
        if (this.connectionStatusText) {
            this.connectionStatusText.textContent = isConnected ? '已连接' : '未连接';
        }

        // 控制遮罩层显示
        if (this.connectionOverlay) {
            this.connectionOverlay.style.display = isConnected ? 'none' : 'flex';
        }

        // 控制功能按钮状态
        this.resetSettingsBtn.disabled = !isConnected;
        this.reloadSettingsBtn.disabled = !isConnected;
        this.saveSettingsBtn.disabled = !isConnected;
    }

    showStatus(message, type = 'info') {
        console.log(`[${type.toUpperCase()}] ${message}`);
    }

    showCustomAlert(title, message) {
        this.alertTitle.textContent = title;
        this.alertMessage.textContent = message;
        this.customAlert.style.display = 'flex';
    }

    hideCustomAlert() {
        this.customAlert.style.display = 'none';
    }

    getBrowserInfo() {
        const ua = navigator.userAgent;
        let browser = '未知浏览器';
        let version = '未知版本';

        if (ua.indexOf('Chrome') > -1) {
            browser = 'Chrome';
            version = ua.match(/Chrome\/(\d+)/)[1];
        } else if (ua.indexOf('Edge') > -1) {
            browser = 'Edge';
            version = ua.match(/Edge\/(\d+)/)[1];
        } else if (ua.indexOf('Firefox') > -1) {
            browser = 'Firefox';
            version = ua.match(/Firefox\/(\d+)/)[1];
        } else if (ua.indexOf('Safari') > -1 && ua.indexOf('Chrome') === -1) {
            browser = 'Safari';
            version = ua.match(/Version\/(\d+)/)[1];
        } else if (ua.indexOf('Opera') > -1 || ua.indexOf('OPR') > -1) {
            browser = 'Opera';
            version = ua.match(/OPR\/(\d+)|Opera\/(\d+)/)[1] || ua.match(/OPR\/(\d+)|Opera\/(\d+)/)[2];
        }

        return {
            browser, version,
            isMobile: /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(ua),
            protocol: window.location.protocol,
            supportsSerial: 'serial' in navigator
        };
    }

    displayBrowserInfo() {
        const info = this.getBrowserInfo();
        const supportStatus = info.supportsSerial ? '✅ 支持' : '❌ 不支持';
        const mobileStatus = info.isMobile ? '📱 移动版' : '💻 桌面版';
        const protocolStatus = info.protocol === 'https:' || info.protocol === 'http:' && window.location.hostname === 'localhost' ? '✅ 符合要求' : '❌ 不符合要求';

        this.browserInfo.innerHTML = `
            <strong>当前浏览器信息：</strong><br>
            - 浏览器：${info.browser} ${info.version}<br>
            - Web Serial API：${supportStatus}<br>
            - 设备类型：${mobileStatus}<br>
            - 访问协议：${info.protocol} (${protocolStatus})<br>
            - 主机名：${window.location.hostname}
        `;
    }

    checkBrowserSupport() {
        const isSupported = 'serial' in navigator;
        if (!isSupported) {
            this.browserSupportAlert.style.display = 'block';
            this.displayBrowserInfo();
            this.connectToggleBtn.disabled = true;
            return false;
        }
        this.browserSupportAlert.style.display = 'none';
        return true;
    }
}

window.addEventListener('DOMContentLoaded', () => new SerialAssistant());
