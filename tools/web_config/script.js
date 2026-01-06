class SerialAssistant {
    constructor() {
        // 命令常量
        this.COMMANDS = {
            CONFIG_MODE_ENABLE: 'config_mode_enabled',
            CONFIG_MODE_DISABLE: 'config_mode_disabled',
            LOAD_SETTINGS: 'load_settings',
            SAVE_SETTINGS: 'save_settings',
            RESET_SETTINGS: 'reset_settings',
            SET_PREFIX: 'set_',

            SHOW_MENU: 'show_menu',
            CLICK: 'click',
            ROTATE_LEFT: 'rotate_left',
            ROTATE_RIGHT: 'rotate_right',
        };

        // 响应常量
        this.RESPONSES = {
            CONFIG_MODE_ENABLED: 'config_mode_enabled_success',
            CONFIG_MODE_DISABLED: 'config_mode_disabled_success',
            LOAD_SETTINGS: 'load_settings_success',
            SAVE_SETTINGS: 'save_settings_success',
            RESET_SETTINGS: 'reset_settings_success',
        };

        this.port = null;
        this.reader = null;
        this.writer = null;
        this.isConnected = false;
        this.connectionCheckInterval = null;
        this.usbVendorId = 0x1209;
        this.receiveBuffer = '';
        this.dataReceivedResolver = null;

        // 设置参数
        this.configParams = {
            led_count: { label: 'LED灯珠数量', type: 'number', min: 1, max: 10, value: 4 },
            color_order: { label: 'LED颜色顺序', type: 'select', options: [{ value: 0, label: 'GRB' }, { value: 1, label: 'RGB' }], value: 0 },
            brightness: { label: '亮度等级', type: 'number', min: 0, max: 4, value: 1 },
            effect_mode: { label: 'LED灯效模式', type: 'number', min: 0, max: 1, value: 0 },
            effect_tick: { label: 'LED灯效循环周期(ms)', type: 'number', min: 20, max: 500, value: 50 },
            rotate_cw: { label: '顺时针旋转角度', type: 'number', min: 1, max: 360, value: 10 },
            rotate_ccw: { label: '逆时针旋转角度', type: 'number', min: -360, max: -1, value: -10 }
        };

        this.init_elements();
        this.init_event_listeners();
        this.check_browser_support();
        this.generate_config_controls();
        this.update_ui_states();
    }

    /**
     * 清除定时器
     * 用于清除连接检查定时器和等待数据定时器
     */
    clear_timer() {
        if (this.connectionCheckInterval) {
            clearInterval(this.connectionCheckInterval);
            this.connectionCheckInterval = null;
        }

        if (this.waitForDataTimer) {
            clearTimeout(this.waitForDataTimer);
            this.waitForDataTimer = null;
        }
    }


    // #region 页面初始化相关方法
    /**
     * 初始化DOM元素引用
     */
    init_elements() {
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

    /**
     * 初始化事件监听器
     */
    init_event_listeners() {
        this.connectToggleBtn.addEventListener('click', () => this.isConnected ? this.serial_disconnect() : this.serial_connect());
        this.resetSettingsBtn.addEventListener('click', () => this.config_reset_settings());
        this.reloadSettingsBtn.addEventListener('click', () => this.config_load_settings());
        this.saveSettingsBtn.addEventListener('click', () => this.config_save_settings());
        this.alertOkBtn.addEventListener('click', () => this.hideCustomAlert());
    }

    /**
     * 生成参数设置的HTML控件
     */
    generate_config_controls() {
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
    // #endregion 页面初始化相关方法


    // #region 串口连接相关方法
    /**
     * 连接串口
     */
    async serial_connect() {
        try {
            await this.serial_cleanup_resources();
            this.showStatus('正在连接串口...', 'success');

            const config = {
                baudRate: 115200,
                dataBits: 8,
                stopBits: 1,
                parity: 'none'
            };
            console.log('串口设置:', config);

            let port = await navigator.serial.requestPort({ filters: [{ usbVendorId: this.usbVendorId }] });

            await port.open(config);
            this.port = port;

            this.showStatus(`串口连接成功 (${config.baudRate} bps, ${config.dataBits}N${config.stopBits}, ${config.parity})`, 'success');

            // 开始接收数据
            this.serial_start_reading();

            // 启用参数设置模式
            await this.config_enable_config_mode();

            // 加载参数设置数据
            await this.config_load_settings();

            this.update_ui_states();

            this.connectionCheckInterval = setInterval(() => this.serial_check_connection_status(), 5000);
        } catch (error) {
            if (error.name === 'NotFoundError') {
                this.showStatus('未选择串口，请重新连接并选择有效串口', 'warning');
                return;
            }
            const errorMessage = this.handle_serial_connection_error(error);
            this.showStatus(errorMessage, 'error');
            console.error('连接串口错误:', error);
        }
    }

    /**
     * 断开串口连接
     */
    async serial_disconnect() {
        try {
            this.showStatus('正在断开串口...', 'warning');
            await this.serial_cleanup_resources();
            this.showStatus('串口已断开', 'success');
        } catch (error) {
            console.error('断开串口错误:', error);
            this.isConnected = false;
            this.update_ui_states();
            this.showStatus(`断开失败: ${error.message}`, 'error');
        }
    }

    /**
     * 开始循环接收串口数据
     */
    async serial_start_reading() {
        if (!this.port || !this.port.readable) {
            return;
        }

        try {
            const textDecoder = new TextDecoderStream('latin1');
            this.reader = this.port.readable.pipeThrough(textDecoder).getReader();

            while (true) {
                const { value, done } = await this.reader.read();
                if (done) break;

                if (value) {
                    // 将文本数据添加到缓冲区
                    this.receiveBuffer += value;
                    this.processReceivedData();
                }
            }
        } catch (error) {
            console.error('接收数据错误:', error);
            if (!this.isConnected) return;
            this.showStatus('接收数据异常，已断开连接', 'error');
            await this.serial_disconnect();
        } finally {
            this.reader = this.serial_release_resource(this.reader);
        }
    }

    /**
     * 获取串口写入器
     * @returns {object} - 串口写入器对象
     */
    serial_get_writer() {
        if (!this.port || !this.port.writable) {
            throw new Error('串口已断开连接');
        }
        if (!this.writer) {
            this.writer = this.port.writable.getWriter();
        }
        return this.writer;
    }

    /**
     * 等待接收数据
     * @param {number} timeout - 超时时间（毫秒）
     * @returns {Promise<string>} 接收到的数据
     */
    serial_wait_for_data(timeout = 500) {
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

    /**
     * 检查串口连接状态
     */
    serial_check_connection_status() {
        if (!this.isConnected) {
            this.clear_timer();
            return;
        }
        if (!this.port || !this.port.readable) {
            this.handle_serial_connection_lost();
        }
    }

    /**
     * 释放串口资源
     * @param {object} resource - 要释放的串口资源（reader, writer）
     */
    serial_release_resource(resource) {
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

    /**
     * 清理串口资源
     */
    async serial_cleanup_resources() {
        this.isConnected = false;
        this.update_ui_states();
        this.clear_timer();
        this.reader = this.serial_release_resource(this.reader);
        this.writer = this.serial_release_resource(this.writer);

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

    /**
     * 处理串口连接错误
     * @param {object} error - 串口连接错误对象
     * @returns {string} - 错误信息字符串
     */
    handle_serial_connection_error(error) {
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

    /**
     * 处理串口连接丢失
     */
    handle_serial_connection_lost() {
        if (this.isConnected) {
            this.isConnected = false;
            this.clear_timer();
            this.update_ui_states();
            this.showStatus('串口连接已断开 (设备可能已复位)', 'error');
            this.showCustomAlert('连接断开', '串口连接已断开，设备可能已复位，请重新连接');
        }
    }
    // #endregion 串口连接相关方法


    // #region 设备参数数据处理相关方法
    /**
     * 处理接收到的数据
    */
    processReceivedData() {
        // 分割接收到的数据，按行处理
        const lines = this.receiveBuffer.split('\r\n');
        this.receiveBuffer = lines.pop(); // 保留最后一行（可能不完整）

        for (const line of lines) {
            let processedLine;

            if (line) {
                // 检查是否是参数设置数据
                if (line.includes('=')) {
                    if (line.startsWith('config=')) {
                        // 处理二进制参数设置数据
                        // 不要trim()，否则会丢失二进制数据
                        processedLine = this.RESPONSES.LOAD_SETTINGS;
                        this.parseConfigBinary(line);
                    } else {
                        // 处理普通key=value格式数据
                        processedLine = line.trim();

                        const [key, value] = processedLine.split('=');
                        const paramKey = key.trim();
                        const paramValue = value.trim();

                        // 更新参数设置
                        if (this.configParams[paramKey]) {
                            this.configParams[paramKey].value = parseInt(paramValue);
                            // 更新UI控件
                            this.update_config_controls(paramKey, parseInt(paramValue));
                        }
                    }
                } else {
                    // 处理不包含=号的响应（如命令确认信息）
                    processedLine = line.trim();
                }
            }

            // 通知等待数据的promise
            if (this.dataReceivedResolver && processedLine) {
                this.dataReceivedResolver(processedLine);
            }
        }
    }

    /**
     * 解析二进制参数设置数据
     * @param {string} data - 包含参数设置数据的字符串
     */
    parseConfigBinary(data) {
        // 提取config=后的二进制数据
        const configData = data.substring('config='.length);

        if (configData.length < 32) {
            console.warn('参数设置数据长度不足32字节:', configData.length);
            return;
        }

        // 创建DataView来解析二进制数据
        const buffer = new ArrayBuffer(32);
        const view = new DataView(buffer);

        // 将字符串转换为二进制数据
        for (let i = 0; i < 32; i++) {
            view.setUint8(i, configData.charCodeAt(i));
        }

        // 解析参数设置数据（小端字节序）
        const config = {
            version: view.getUint8(0),
            revision: view.getUint8(1),
            led_count: view.getUint8(2),
            color_order: view.getUint8(3),
            brightness: view.getUint8(4),
            effect_mode: view.getUint8(5),
            effect_tick: view.getUint16(6, true), // true表示小端字节序
            rotate_cw: view.getInt16(8, true),
            rotate_ccw: view.getInt16(10, true),
            // reserved字段从16-31，共16字节，暂不处理
        };

        // 更新参数设置
        this.configParams.led_count.value = config.led_count;
        this.configParams.color_order.value = config.color_order;
        this.configParams.brightness.value = config.brightness;
        this.configParams.effect_mode.value = config.effect_mode;
        this.configParams.effect_tick.value = config.effect_tick;
        this.configParams.rotate_cw.value = config.rotate_cw;
        this.configParams.rotate_ccw.value = config.rotate_ccw;

        // 更新UI控件
        this.update_config_controls('led_count', config.led_count);
        this.update_config_controls('color_order', config.color_order);
        this.update_config_controls('brightness', config.brightness);
        this.update_config_controls('effect_mode', config.effect_mode);
        this.update_config_controls('effect_tick', config.effect_tick);
        this.update_config_controls('rotate_cw', config.rotate_cw);
        this.update_config_controls('rotate_ccw', config.rotate_ccw);

        // 显示固件版本
        if (this.firmwareVersion) {
            this.firmwareVersion.textContent = `${config.version}.${config.revision}`;
        }

        console.log("Config:", config);
    }
    // #endregion 设备参数数据处理相关方法


    // #region 设备参数设置相关方法
    /**
     * 启用参数设置模式
     */
    async config_enable_config_mode() {
        try {
            this.showStatus('正在启用参数设置模式...', 'info');

            const writer = this.serial_get_writer();
            const command = this.COMMANDS.CONFIG_MODE_ENABLE + '\n';
            const buffer = new TextEncoder().encode(command);
            await writer.write(buffer);

            const response = await this.serial_wait_for_data();

            if (response === this.RESPONSES.CONFIG_MODE_ENABLED) {
                this.isConnected = true;
                this.showStatus('参数设置模式已成功启用', 'success');
            } else {
                throw new Error(`意外响应: ${response}`);
            }
        } catch (error) {
            this.writer = this.serial_release_resource(this.writer);
            this.showStatus(`启用参数设置模式失败: ${error.message}`, 'error');
        }
    }

    /**
     * 加载参数设置
     */
    async config_load_settings() {
        try {
            this.showStatus('正在加载参数设置...', 'info');

            const writer = this.serial_get_writer();
            const command = this.COMMANDS.LOAD_SETTINGS + '\n';
            const buffer = new TextEncoder().encode(command);
            await writer.write(buffer);

            const response = await this.serial_wait_for_data();

            if (response === this.RESPONSES.LOAD_SETTINGS) {
                this.showStatus('参数设置已加载', 'success');
            } else {
                throw new Error(`意外响应: ${response}`);
            }
        } catch (error) {
            this.showStatus(`加载参数设置失败: ${error.message}`, 'error');
        }
    }

    async config_save_settings() {
        // try {
            const writer = this.serial_get_writer();

            // 创建配置数据缓冲区（共30字节，不含version和revision）
            const buffer = new ArrayBuffer(30);
            const view = new DataView(buffer);

            // 按照config_t结构体的顺序写入数据（跳过version和revision）
            let offset = 0;

            // led_count (1字节)
            view.setUint8(offset++, this.configParams.led_count.value);

            // color_order (1字节)
            view.setUint8(offset++, this.configParams.color_order.value);

            // brightness (1字节)
            view.setUint8(offset++, this.configParams.brightness.value);

            // effect_mode (1字节)
            view.setUint8(offset++, this.configParams.effect_mode.value);

            // effect_tick (2字节，小端)
            view.setUint16(offset, this.configParams.effect_tick.value, true);
            offset += 2;

            // rotate_cw (2字节，小端)
            view.setInt16(offset, this.configParams.rotate_cw.value, true);
            offset += 2;

            // rotate_ccw (2字节，小端)
            view.setInt16(offset, this.configParams.rotate_ccw.value, true);
            offset += 2;

            // reserved字段：使用缓冲区剩余的大小填充
            const max_offset = buffer.byteLength;
            const reserved_size = max_offset - offset;
            for (let i = 0; i < reserved_size; i++) {
                view.setUint8(offset + i, 0);
            }

            // 构建完整命令："save_settings=" + 30字节二进制数据 + "\n"
            const commandPrefix = this.COMMANDS.SAVE_SETTINGS + '=';
            const newlineBuffer = new TextEncoder().encode('\n');
            const prefixBuffer = new TextEncoder().encode(commandPrefix);
            const dataBuffer = new Uint8Array(buffer); // 将ArrayBuffer转换为Uint8Array
            
            // 使用实际编码后的字节数创建缓冲区
            const fullBuffer = new Uint8Array(prefixBuffer.length + dataBuffer.length + newlineBuffer.length);
            
            fullBuffer.set(prefixBuffer, 0);
            fullBuffer.set(dataBuffer, prefixBuffer.length);
            fullBuffer.set(newlineBuffer, prefixBuffer.length + dataBuffer.length);
            
            // 一次性发送完整命令
            await writer.write(fullBuffer);

            const response = await this.serial_wait_for_data();

            if (response === this.RESPONSES.SAVE_SETTINGS) {
                this.showStatus('设置已保存到设备', 'success');
            } else {
                throw new Error(`意外响应: ${response}`);
            }
        // } catch (error) {
        //     this.writer = this.serial_release_resource(this.writer);
        //     this.showStatus(`保存设置失败: ${error.message}`, 'error');
        // }
    }

    /**
     * 恢复默认参数设置
     */
    async config_reset_settings() {
        try {
            this.showStatus('正在重置参数设置...', 'info');

            const writer = this.serial_get_writer();
            const command = this.COMMANDS.RESET_SETTINGS + '\n';
            const buffer = new TextEncoder().encode(command);
            await writer.write(buffer);

            const response = await this.serial_wait_for_data();

            if (response === this.RESPONSES.RESET_SETTINGS) {
                this.showStatus('参数设置已重置为默认值', 'success');

                // 加载参数设置
                await this.config_load_settings();
            } else {
                throw new Error(`意外响应: ${response}`);
            }
        } catch (error) {
            this.showStatus(`重置参数设置失败: ${error.message}`, 'error');
        }
    }
    // #endregion 设备参数设置相关方法


    // #region 页面控件更新相关方法
    /**
     * 更新参数设置控件的值
     * @param {string} key - 参数键名
     * @param {number} value - 参数值
     */
    update_config_controls(key, value) {
        const control = document.getElementById(`config-${key}`);
        if (control) {
            control.value = value;
        }
    }

    /**
     * 更新UI状态
     * 包括连接按钮文本、指示灯颜色、遮罩层显示和功能按钮状态
     */
    update_ui_states() {
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
    // #endregion 页面控件更新相关方法


    // #region 状态显示和弹窗相关方法
    /**
     * 显示状态消息
     * @param {string} message - 状态消息内容
     * @param {string} type - 状态类型，可选值：'info'（默认）、'success'、'error'
     */
    showStatus(message, type = 'info') {
        console.log(`[${type.toUpperCase()}] ${message}`);
    }

    /**
     * 显示自定义弹窗
     * @param {string} title - 弹窗标题
     * @param {string} message - 弹窗消息内容
     */
    showCustomAlert(title, message) {
        this.alertTitle.textContent = title;
        this.alertMessage.textContent = message;
        this.customAlert.style.display = 'flex';
    }

    /**
     * 隐藏自定义弹窗
     */
    hideCustomAlert() {
        this.customAlert.style.display = 'none';
    }
    // #endregion 状态显示和弹窗相关方法


    // #region 浏览器信息相关方法
    /**
     * 获取浏览器信息
     * 包括浏览器类型、版本、是否支持Web Serial API、设备类型和访问协议
     * @returns {object} 浏览器信息对象
     */
    __get_browser_info() {
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

    /**
     * 显示浏览器信息
     * 包括浏览器类型、版本、是否支持Web Serial API、设备类型和访问协议
     */
    __display_browser_info() {
        const info = this.__get_browser_info();
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

    /**
     * 检查浏览器是否支持Web Serial API
     * 如果不支持，显示警告并禁用连接按钮
     * @returns {boolean} 是否支持
     */
    check_browser_support() {
        const isSupported = 'serial' in navigator;
        if (!isSupported) {
            this.browserSupportAlert.style.display = 'block';
            this.__display_browser_info();
            this.connectToggleBtn.disabled = true;
            return false;
        }
        this.browserSupportAlert.style.display = 'none';
        return true;
    }
    // #endregion 浏览器信息相关方法
}

window.serialAssistant = null;
window.addEventListener('DOMContentLoaded', () => {
    window.serialAssistant = new SerialAssistant();
});
