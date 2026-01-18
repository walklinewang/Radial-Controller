class SerialAssistant {
    constructor() {
        // 命令常量
        this.COMMANDS = {
            CONFIG_MODE_ENABLE: 'config_mode_enabled',
            LOAD_SETTINGS: 'load_settings',
            SAVE_SETTINGS: 'save_settings',
            RESET_SETTINGS: 'reset_settings',
            SET_PREFIX: 'set_',

            SHOW_MENU: 'show_menu',
            CLICK: 'click',
            ROTATE_LEFT: 'rotate_left',
            ROTATE_RIGHT: 'rotate_right',
            HEARTBEAT: 'heartbeat',
        };

        // 命令响应常量
        this.RESPONSES = {
            CONFIG_MODE_ENABLED_SUCCESS: 'config_mode_enabled_success',
            CONFIG_MODE_TIMEDOUT: 'config_mode_timedout',
            LOAD_SETTINGS_SUCCESS: 'load_settings_success',
            SAVE_SETTINGS_SUCCESS: 'save_settings_success',
            SAVE_SETTINGS_FAILED: 'save_settings_failed',
            RESET_SETTINGS_SUCCESS: 'reset_settings_success',
        };

        // USB设备供应商ID，用于筛选目标设备
        this.USB_VENDOR_ID = 0x1209;

        this.port = null;
        this.reader = null;
        this.writer = null;
        this.is_connected = false;
        this.connection_check_timer = null;
        this.received_buffer = new Uint8Array(0);
        this.data_received_resolver = null;

        // 心跳包相关设置
        this.HEARTBEAT_INTERVAL = 2000; // 心跳发送间隔（2秒）
        this.heartbeat_timer = null; // 心跳定时器
        this.last_heartbeat_time = Date.now();

        // 设置参数
        this.config_params = {
            led_count: { label: '灯珠数量', type: 'slider', min: 1, max: 10, step: 1, value: 4 },
            brightness: { label: '亮度等级', type: 'slider', min: 0, max: 4, step: 1, value: 3, displayValueOffset: 1 },
            color_order: { label: '颜色顺序', type: 'select', options: [{ value: 0, label: 'GRB (默认)' }, { value: 1, label: 'RGB' }], value: 0 },
            effect_mode: { label: '灯效模式', type: 'select', options: [{ value: 0, label: '默认' }], value: 0 },
            rotate_interval: { label: '流动灯效循环周期 (毫秒)', type: 'number', min: 20, max: 500, value: 50 },
            fade_duration: { label: '渐变灯效持续时间 (毫秒)', type: 'number', min: 100, max: 300, value: 150 },
            rotate_cw: { label: '顺时针旋转角度', type: 'number', min: 1, max: 360, value: 10 },
            rotate_ccw: { label: '逆时针旋转角度', type: 'number', min: -360, max: -1, value: -10 },
            step_per_teeth: { label: '每转动一齿触发动作次数', type: 'select', options: [{ value: 1, label: '1' }, { value: 2, label: '2 (默认)' }], value: 2 },
            phase: { label: '相位', type: 'select', options: [{ value: 0, label: '正向脉冲 (默认)' }, { value: 1, label: '反向脉冲' }], value: 0 },
        };

        this.init_static_elements();
        this.generate_config_controls();
        this.check_browser_support();
        this.update_ui_states();
    }

    /**
     * 清除定时器
     * 用于清除连接检查定时器、等待数据定时器和心跳定时器
     */
    clear_timer() {
        if (this.connection_check_timer) {
            clearInterval(this.connection_check_timer);
            this.connection_check_timer = null;
        }

        if (this.wait_for_data_timer) {
            clearTimeout(this.wait_for_data_timer);
            this.wait_for_data_timer = null;
        }

        if (this.heartbeat_timer) {
            clearInterval(this.heartbeat_timer);
            this.heartbeat_timer = null;
        }
    }


    // #region 页面初始化相关方法
    /**
     * 初始化DOM元素引用和事件监听器
     */
    init_static_elements() {
        this.browser_support_alert = document.getElementById('browser-support-alert');
        this.browser_info = document.getElementById('browser-info');
        this.connect_toggle_button = document.getElementById('connect-toggle-btn');
        this.connection_status_icon = document.getElementById('connection-status-icon');
        this.connection_status_text = document.getElementById('connection-status-text');
        this.firmware_version = document.getElementById('firmware-version');
        this.config_container = document.getElementById('config-container');
        this.reset_settings_button = document.getElementById('reset-settings-btn');
        this.reload_settings_button = document.getElementById('reload-settings-btn');
        this.save_settings_button = document.getElementById('save-settings-btn');
        this.connection_overlay = document.getElementById('connection-overlay');
        this.custom_alert = document.getElementById('custom-alert');
        this.alert_title = document.getElementById('alert-title');
        this.alert_message = document.getElementById('alert-message');
        this.alert_ok_button = document.getElementById('alert-ok');
        this.alert_overlay = this.custom_alert.querySelector('.custom-alert-overlay');

        this.connect_toggle_button.addEventListener('click', () => this.is_connected ? this.serial_disconnect() : this.serial_connect());
        this.reset_settings_button.addEventListener('click', () => this.config_reset_settings());
        this.reload_settings_button.addEventListener('click', () => this.config_load_settings());
        this.save_settings_button.addEventListener('click', () => this.config_save_settings());
        this.alert_ok_button.addEventListener('click', () => this.hideCustomAlert());
        this.alert_overlay.addEventListener('click', () => this.hideCustomAlert());

        // 防止点击弹窗内容时关闭弹窗
        const alert_content = this.custom_alert.querySelector('.custom-alert-content');
        alert_content.addEventListener('click', (e) => e.stopPropagation());
    }

    /**
     * 创建参数设置输入控件
     * @param {string} paramKey - 参数键名
     * @param {object} param - 参数设置对象
     * @param {HTMLElement} container - 容器元素
     */
    __create_config_input(paramKey, param, container) {
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
                // 确保类型匹配，将option.value转换为数字进行比较
                if (parseInt(option.value) === param.value) {
                    opt.selected = true;
                }
                input.appendChild(opt);
            });
        } else if (param.type === 'slider') {
            // 创建滑块容器
            const sliderContainer = document.createElement('div');
            sliderContainer.className = 'slider-container';

            // 创建滑块
            input = document.createElement('input');
            input.type = 'range';
            input.id = `config-${paramKey}`;
            input.name = paramKey;
            input.value = param.value;
            input.min = param.min;
            input.max = param.max;
            input.step = param.step || '1';

            // 创建数值显示
            const valueDisplay = document.createElement('span');
            valueDisplay.className = 'slider-value';
            // 考虑显示偏移量（用于亮度等级等需要显示值与实际存储值不同的情况）
            const displayValue = param.displayValueOffset ? param.value + param.displayValueOffset : param.value;
            valueDisplay.textContent = displayValue;

            sliderContainer.appendChild(input);
            sliderContainer.appendChild(valueDisplay);
            container.appendChild(sliderContainer);
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

        // 实时过滤非法字符（仅在输入时进行格式控制，不进行范围验证）
        if (param.type === 'number') {
            input.addEventListener('input', (e) => {
                let value = e.target.value;
                // 只允许输入数字和负号（仅在开头允许一个负号）
                value = value.replace(/[^0-9-]/g, '');
                // 确保负号只在开头出现一次
                if (value.indexOf('-') > 0) {
                    value = value.replace(/-/g, '');
                }
                e.target.value = value;
            });
        }

        // 滑块仍然需要实时更新显示
        if (param.type === 'slider') {
            input.addEventListener('input', (e) => {
                let value = parseInt(e.target.value);
                // 更新滑块的数值显示
                const valueDisplay = e.target.nextElementSibling;
                if (valueDisplay && valueDisplay.className === 'slider-value') {
                    // 考虑显示偏移量
                    const displayValue = param.displayValueOffset ? value + param.displayValueOffset : value;
                    valueDisplay.textContent = displayValue;
                }
                // 更新参数值
                this.config_params[paramKey].value = value;
            });
        }

        // Select下拉框实时更新
        if (param.type === 'select') {
            input.addEventListener('change', (e) => {
                this.config_params[paramKey].value = parseInt(e.target.value);
            });
        }

        // 在输入完成（失去焦点）时进行范围验证
        input.addEventListener('blur', (e) => {
            let value = e.target.value;

            // 如果是数字类型，确保值在有效范围内
            if (param.type === 'number') {
                value = parseInt(value);

                // 验证最小值
                if (param.min !== undefined && (isNaN(value) || value < param.min)) {
                    value = param.min;
                    e.target.value = value;
                    this.show_status(`参数 ${param.label} 最小值为 ${param.min}`, 'warning');
                }

                // 验证最大值
                if (param.max !== undefined && value > param.max) {
                    value = param.max;
                    e.target.value = value;
                    this.show_status(`参数 ${param.label} 最大值为 ${param.max}`, 'warning');
                }

                // 更新参数值
                this.config_params[paramKey].value = value;
            }
        });

        if (param.type !== 'slider') {
            container.appendChild(input);
        }
    }

    /**
     * 生成参数设置的HTML控件
     */
    generate_config_controls() {
        if (!this.config_container) return;

        this.config_container.innerHTML = '';

        // 分组定义参数
        const ledParams = ['led_count', 'brightness', 'color_order', 'effect_mode', 'rotate_interval', 'fade_duration'];
        const encoderParams = ['rotate_cw', 'rotate_ccw', 'step_per_teeth', 'phase'];

        // 创建LED相关参数容器
        const ledContainer = document.createElement('div');
        ledContainer.className = 'config-group led-group';

        // 添加LED参数组标题
        const ledTitle = document.createElement('h3');
        ledTitle.className = 'config-group-title';
        ledTitle.textContent = 'LED设置';
        ledContainer.appendChild(ledTitle);

        // 创建编码器相关参数容器
        const encoderContainer = document.createElement('div');
        encoderContainer.className = 'config-group encoder-group';

        // 添加编码器参数组标题
        const encoderTitle = document.createElement('h3');
        encoderTitle.className = 'config-group-title';
        encoderTitle.textContent = '编码器设置';
        encoderContainer.appendChild(encoderTitle);

        // 添加LED相关参数
        ledParams.forEach(paramKey => {
            const param = this.config_params[paramKey];
            const container = document.createElement('div');
            container.className = 'config-item';
            this.__create_config_input(paramKey, param, container);
            ledContainer.appendChild(container);
        });

        // 添加编码器相关参数
        encoderParams.forEach(paramKey => {
            const param = this.config_params[paramKey];
            const container = document.createElement('div');
            container.className = 'config-item';
            this.__create_config_input(paramKey, param, container);
            encoderContainer.appendChild(container);
        });

        // 将所有容器添加到主容器
        this.config_container.appendChild(ledContainer);
        this.config_container.appendChild(encoderContainer);
    }
    // #endregion 页面初始化相关方法


    // #region 串口连接相关方法
    /**
     * 连接串口
     */
    async serial_connect() {
        try {
            await this.serial_cleanup_resources();
            this.show_status('正在连接串口...', 'success');

            const serial_config = {
                baudRate: 115200,
                dataBits: 8,
                stopBits: 1,
                parity: 'none'
            };
            this.show_status('串口设置:');
            this.show_status(serial_config);

            let port = await navigator.serial.requestPort({ filters: [{ usbVendorId: this.USB_VENDOR_ID }] });

            await port.open(serial_config);
            this.port = port;

            this.show_status(`串口连接成功 (${serial_config.baudRate} bps, ${serial_config.dataBits}N${serial_config.stopBits}, ${serial_config.parity})`, 'success');

            // 开始接收数据
            this.serial_start_reading();

            // 启用参数设置模式
            await this.config_enable_config_mode();

            // 加载参数设置数据
            await this.config_load_settings();

            this.update_ui_states();

            // 缩短连接检查间隔，提高实时性
            this.connection_check_timer = setInterval(() => this.serial_check_connection_status(), 2000);
        } catch (error) {
            if (error.name === 'NotFoundError') {
                this.show_status('未选择串口，请重新连接并选择有效串口', 'warning');
                return;
            }
            const errorMessage = this.handle_serial_connection_error(error);
            this.show_status(errorMessage, 'error');
        }
    }

    /**
     * 断开串口连接
     */
    async serial_disconnect() {
        try {
            this.show_status('正在断开串口...', 'warning');
            await this.serial_cleanup_resources();
            this.show_status('串口已断开', 'success');
        } catch (error) {
            this.is_connected = false;
            this.update_ui_states();
            this.show_status(`断开失败: ${error.message}`, 'error');
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
            this.reader = this.port.readable.getReader();

            while (true) {
                const { value, done } = await this.reader.read();
                if (done) break;

                if (value) {
                    const uint8_array = new Uint8Array(value);

                    // 追加到缓冲区
                    const new_buffer = new Uint8Array(this.received_buffer.length + uint8_array.length);
                    new_buffer.set(this.received_buffer);
                    new_buffer.set(uint8_array, this.received_buffer.length);
                    this.received_buffer = new_buffer;
                    this.process_received_data();
                }
            }
        } catch (error) {
            if (!this.is_connected) return;

            this.show_status('接收数据异常，已断开连接', 'error');
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
     * @param {Array<string>} allowed_responses - 允许的响应类型数组，如果未提供则接收所有响应
     * @returns {Promise<string>} 接收到的数据
     */
    serial_wait_for_data(timeout = 500, allowed_responses = null) {
        return new Promise((resolve, reject) => {
            const original_resolver = this.data_received_resolver;

            this.data_received_resolver = (data) => {
                // 如果没有指定允许的响应类型，或者接收到的响应在允许列表中，则返回该响应
                if (!allowed_responses || allowed_responses.includes(data)) {
                    this.data_received_resolver = original_resolver;
                    resolve(data);
                } else {
                    // 忽略不符合条件的响应
                    this.show_status(`忽略不相关响应: ${data}`);
                }
            };

            // 设置等待超时
            this.wait_for_data_timer = setTimeout(() => {
                this.data_received_resolver = original_resolver;
                reject(new Error('等待数据超时'));
            }, timeout);
        });
    }

    /**
     * 检查串口连接状态
     */
    serial_check_connection_status() {
        if (!this.is_connected) {
            this.clear_timer();
            return;
        }

        // 检查端口是否存在
        if (!this.port) {
            this.handle_serial_connection_lost();
            return;
        }
    }

    /**
     * 释放串口资源
     * @param {object} resource - 要释放的串口资源（reader, writer）
     */
    serial_release_resource(resource) {
        if (resource && typeof resource === 'object') {
            try {
                // 检查是否有releaseLock方法
                if (typeof resource.releaseLock === 'function') {
                    resource.releaseLock();
                }
                // 检查是否有close方法
                else if (typeof resource.close === 'function') {
                    resource.close();
                }
            } catch (error) {
                this.show_status(`释放资源失败: ${error.message}`, 'warning');
            }
        }
        return null;
    }

    /**
     * 清理串口资源
     */
    async serial_cleanup_resources() {
        this.is_connected = false;
        this.update_ui_states();
        this.clear_timer();

        // 首先释放reader和writer资源
        if (this.reader) {
            try {
                if (typeof this.reader.cancel === 'function') {
                    await this.reader.cancel();
                }
            } catch (error) {
                this.show_status(`取消reader失败: ${error.message}`, 'warning');
            }
            this.reader = this.serial_release_resource(this.reader);
        }

        if (this.writer) {
            try {
                if (typeof this.writer.close === 'function') {
                    await this.writer.close();
                }
            } catch (error) {
                this.show_status(`关闭writer失败: ${error.message}`, 'warning');
            }
            this.writer = this.serial_release_resource(this.writer);
        }

        // 重置接收缓冲区和数据解析器
        this.received_buffer = new Uint8Array(0);
        this.data_received_resolver = null;

        if (this.port) {
            try {
                // 等待一小段时间确保资源完全释放
                await new Promise(resolve => setTimeout(resolve, 100));
                await this.port.close();
            } catch (error) {
                // 忽略"Cannot cancel a locked stream"错误，因为这通常是因为流已经被释放
                if (!error.message.includes('locked stream')) {
                    this.show_status(`关闭端口失败: ${error.message}`, 'warning');
                }
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
            this.show_custom_alert(errorInfo.title, errorInfo.content);
            return `串口连接失败: ${errorInfo.content}`;
        }

        this.show_custom_alert('连接失败', error.message);
        return `连接失败: ${error.message}`;
    }

    /**
     * 处理串口连接丢失
     */
    handle_serial_connection_lost() {
        if (this.is_connected) {
            this.is_connected = false;
            this.clear_timer();
            this.update_ui_states();
            this.show_status('串口连接已断开 (设备可能已复位)', 'error');
            this.show_custom_alert('连接断开', '串口连接已断开，设备可能已复位，请重新连接');
        }
    }

    /**
     * 处理参数设置模式超时
     */
    handle_config_mode_timedout() {
        this.is_connected = false;
        this.clear_timer();
        this.update_ui_states();
        this.show_status('参数设置模式已超时退出', 'error');
        this.show_custom_alert('连接断开', '参数设置模式已超时退出，请重新连接');
    }
    // #endregion 串口连接相关方法


    // #region 设备参数数据处理相关方法
    /**
     * 解析二进制参数设置数据
     * @param {Uint8Array} data - 包含参数设置数据的Uint8Array
     */
    __parse_config_binary_data(data) {
        if (data.length < 32) {
            this.show_status(`参数设置数据长度不足32字节: ${data.length}`, 'warning');
            return;
        }

        // 使用Uint8Array创建DataView
        const view = new DataView(data.buffer);

        // 解析参数设置数据（小端字节序）
        const config = {
            version: view.getUint8(0),
            revision: view.getUint8(1),
            led_count: view.getUint8(2),
            color_order: view.getUint8(3),
            brightness: view.getUint8(4),
            effect_mode: view.getUint8(5),
            rotate_interval: view.getUint16(6, true), // true表示小端字节序
            fade_duration: view.getUint16(8, true),
            rotate_cw: view.getInt16(10, true),
            rotate_ccw: view.getInt16(12, true),
            step_per_teeth: view.getUint8(14),
            phase: view.getUint8(15),
            // reserved字段从16-31，共16字节，暂不处理
        };

        // 更新参数设置
        for (const key in config) {
            if (this.config_params[key] !== undefined) {
                let value = config[key];
                const param = this.config_params[key];

                // 验证并修正值
                if (param.type === 'number') {
                    if (param.min !== undefined && value < param.min) {
                        value = param.min;
                    }
                    if (param.max !== undefined && value > param.max) {
                        value = param.max;
                    }
                }

                this.config_params[key].value = value;
            }
        }

        // 更新UI控件
        this.update_config_controls('led_count', this.config_params.led_count.value);
        this.update_config_controls('color_order', this.config_params.color_order.value);
        this.update_config_controls('brightness', this.config_params.brightness.value);
        this.update_config_controls('effect_mode', this.config_params.effect_mode.value);
        this.update_config_controls('rotate_interval', this.config_params.rotate_interval.value);
        this.update_config_controls('fade_duration', this.config_params.fade_duration.value);
        this.update_config_controls('rotate_cw', this.config_params.rotate_cw.value);
        this.update_config_controls('rotate_ccw', this.config_params.rotate_ccw.value);
        this.update_config_controls('step_per_teeth', this.config_params.step_per_teeth.value);
        this.update_config_controls('phase', this.config_params.phase.value);

        // 显示固件版本
        if (this.firmware_version) {
            this.firmware_version.textContent = `${config.version}.${config.revision}`;
        }

        this.show_status('参数设置');
        this.show_status(config);
    }

    /**
     * 处理接收到的数据
    */
    process_received_data() {
        // 循环处理所有完整的行
        while (true) {
            // 查找换行符（LF）
            let lf_index = -1;
            for (let i = 0; i < this.received_buffer.length; i++) {
                if (this.received_buffer[i] === 0x0A) { // LF
                    lf_index = i;
                    break;
                }
            }

            if (lf_index === -1) {
                break; // 没有完整的行，退出循环
            }

            // 提取一行数据（包含CR）
            const line_buffer = this.received_buffer.slice(0, lf_index + 1);

            // 转换为字符串
            const line = new TextDecoder('latin1').decode(line_buffer).trim();

            if (line) {
                let processed_line;

                // 检查是否是参数设置数据
                if (line.includes('=')) {
                    if (line.startsWith('config=')) {
                        // 处理二进制参数设置数据
                        processed_line = this.RESPONSES.LOAD_SETTINGS_SUCCESS;

                        // 提取config=后的32字节二进制数据
                        const config_start = 'config='.length;
                        const config_data = this.received_buffer.slice(config_start, config_start + 32);
                        this.__parse_config_binary_data(config_data);
                    } else {
                        // 处理普通key=value格式数据
                        processed_line = line.trim();

                        const [key, value] = processed_line.split('=');
                        const param_key = key.trim();
                        const param_value = value.trim();

                        // 更新参数设置
                        if (this.config_params[param_key]) {
                            this.config_params[param_key].value = parseInt(param_value);
                            // 更新UI控件
                            this.update_config_controls(param_key, parseInt(param_value));
                        }
                    }
                } else {
                    // 处理不包含=号的响应（如命令确认信息）
                    processed_line = line.trim();
                }

                // 通知等待数据的promise
                let should_handle_timeout = true;

                if (this.data_received_resolver && processed_line) {
                    // 只有当没有等待数据的promise，或者响应不在允许列表中时，才会继续处理超时
                    // 否则，等待数据的promise会处理这个响应
                    this.data_received_resolver(processed_line);
                    should_handle_timeout = false;
                }

                // 处理参数设置模式超时，只有当没有等待数据的promise时才处理
                if (should_handle_timeout && processed_line === this.RESPONSES.CONFIG_MODE_TIMEDOUT) {
                    this.handle_config_mode_timedout();
                }
            }

            // 移除已处理的数据
            this.received_buffer = this.received_buffer.slice(lf_index + 1);
        }
    }
    // #endregion 设备参数数据处理相关方法


    // #region 设备参数设置相关方法
    /**
     * 启用参数设置模式
     */
    async config_enable_config_mode() {
        try {
            this.show_status('正在启用参数设置模式...');

            const writer = this.serial_get_writer();
            const command = this.COMMANDS.CONFIG_MODE_ENABLE + '\n';
            const buffer = new TextEncoder().encode(command);
            await writer.write(buffer);

            const response = await this.serial_wait_for_data(500, [this.RESPONSES.CONFIG_MODE_ENABLED_SUCCESS]);

            if (response === this.RESPONSES.CONFIG_MODE_ENABLED_SUCCESS) {
                this.is_connected = true;
                this.show_status('参数设置模式已成功启用', 'success');

                // 启动心跳定时器，每2秒发送一次心跳包
                this.heartbeat_timer = setInterval(() => this.config_send_heartbeat(), this.HEARTBEAT_INTERVAL);

                // 立即发送第一个心跳包
                await this.config_send_heartbeat();
            } else {
                throw new Error(`意外响应: ${response}`);
            }
        } catch (error) {
            this.writer = this.serial_release_resource(this.writer);
            this.show_status(`启用参数设置模式失败: ${error.message}`, 'error');
        }
    }

    /**
     * 加载参数设置
     */
    async config_load_settings() {
        try {
            this.show_status('正在加载参数设置...');

            const writer = this.serial_get_writer();
            const command = this.COMMANDS.LOAD_SETTINGS + '\n';
            const buffer = new TextEncoder().encode(command);
            await writer.write(buffer);

            const response = await this.serial_wait_for_data(500, [this.RESPONSES.LOAD_SETTINGS_SUCCESS]);

            if (response === this.RESPONSES.LOAD_SETTINGS_SUCCESS) {
                this.show_status('参数设置已加载', 'success');
            } else {
                throw new Error(`意外响应: ${response}`);
            }
        } catch (error) {
            this.show_status(`加载参数设置失败: ${error.message}`, 'error');
        }
    }

    /**
     * 验证所有参数是否在有效范围内
     * @returns {boolean} 所有参数是否有效
     */
    __config_validate_all_params() {
        let is_valid = true;

        // 遍历所有参数
        for (const [key, param] of Object.entries(this.config_params)) {
            if (param.type === 'number' || param.type === 'slider') {
                const value = param.value;

                // 验证最小值
                if (param.min !== undefined && (isNaN(value) || value < param.min)) {
                    this.show_status(`参数 ${param.label} 最小值为 ${param.min}`, 'warning');
                    is_valid = false;
                }

                // 验证最大值
                if (param.max !== undefined && value > param.max) {
                    this.show_status(`参数 ${param.label} 最大值为 ${param.max}`, 'warning');
                    is_valid = false;
                }
            }
        }

        return is_valid;
    }

    /**
     * 保存参数设置
     */
    async config_save_settings() {
        try {
            // 保存前先验证所有参数
            if (!this.__config_validate_all_params()) {
                return; // 参数无效，不执行保存操作
            }

            const writer = this.serial_get_writer();

            // 创建参数设置数据缓冲区（共30字节，不含version和revision）
            const buffer = new ArrayBuffer(30);
            const view = new DataView(buffer);

            // 按照config_t结构体的顺序写入数据（跳过version和revision）
            let offset = 0;

            // led_count (1字节)
            view.setUint8(offset++, this.config_params.led_count.value);

            // color_order (1字节)
            view.setUint8(offset++, this.config_params.color_order.value);

            // brightness (1字节)
            view.setUint8(offset++, this.config_params.brightness.value);

            // effect_mode (1字节)
            view.setUint8(offset++, this.config_params.effect_mode.value);

            // rotate_interval (2字节，小端)
            view.setUint16(offset, this.config_params.rotate_interval.value, true);
            offset += 2;

            // fade_duration (2字节，小端)
            view.setUint16(offset, this.config_params.fade_duration.value, true);
            offset += 2;

            // rotate_cw (2字节，小端)
            view.setInt16(offset, this.config_params.rotate_cw.value, true);
            offset += 2;

            // rotate_ccw (2字节，小端)
            view.setInt16(offset, this.config_params.rotate_ccw.value, true);
            offset += 2;

            // step_per_teeth (1字节)
            view.setUint8(offset++, this.config_params.step_per_teeth.value);

            // phase (1字节)
            view.setUint8(offset++, this.config_params.phase.value);

            // reserved字段：使用缓冲区剩余的大小填充
            const reserved_size = buffer.byteLength - offset;
            for (let i = 0; i < reserved_size; i++) {
                view.setUint8(offset + i, 0);
            }

            // 构建完整命令："save_settings=" + 30字节二进制数据 + "\n"
            const command_prefix = this.COMMANDS.SAVE_SETTINGS + '=';
            const newline_buffer = new TextEncoder().encode('\n');
            const prefix_buffer = new TextEncoder().encode(command_prefix);
            const data_buffer = new Uint8Array(buffer); // 将ArrayBuffer转换为Uint8Array

            // 使用实际编码后的字节数创建缓冲区
            const full_buffer = new Uint8Array(prefix_buffer.length + data_buffer.length + newline_buffer.length);

            full_buffer.set(prefix_buffer, 0);
            full_buffer.set(data_buffer, prefix_buffer.length);
            full_buffer.set(newline_buffer, prefix_buffer.length + data_buffer.length);

            // 一次性发送完整命令
            await writer.write(full_buffer);

            const response = await this.serial_wait_for_data(500, [this.RESPONSES.SAVE_SETTINGS_SUCCESS, this.RESPONSES.SAVE_SETTINGS_FAILED]);

            if (response === this.RESPONSES.SAVE_SETTINGS_SUCCESS) {
                this.show_status('设置已保存到设备', 'success');
                this.show_custom_alert('保存设置成功', "参数设置已保存到设备");
            } else if (response === this.RESPONSES.SAVE_SETTINGS_FAILED) {
                this.show_custom_alert('保存设置失败', "检查参数设置是否正确");
            } else {
                throw new Error(`意外响应: ${response}`);
            }
        } catch (error) {
            this.writer = this.serial_release_resource(this.writer);
            this.show_status(`保存设置失败: ${error.message}`, 'error');
        }
    }

    /**
     * 恢复默认参数设置
     */
    async config_reset_settings() {
        try {
            this.show_status('正在重置参数设置...');

            const writer = this.serial_get_writer();
            const command = this.COMMANDS.RESET_SETTINGS + '\n';
            const buffer = new TextEncoder().encode(command);
            await writer.write(buffer);

            const response = await this.serial_wait_for_data(500, [this.RESPONSES.RESET_SETTINGS_SUCCESS]);

            if (response === this.RESPONSES.RESET_SETTINGS_SUCCESS) {
                this.show_status('参数设置已重置为默认值', 'success');

                // 加载参数设置
                await this.config_load_settings();
            } else {
                throw new Error(`意外响应: ${response}`);
            }
        } catch (error) {
            this.show_status(`重置参数设置失败: ${error.message}`, 'error');
        }
    }

    /**
     * 发送心跳请求
     */
    async config_send_heartbeat() {
        try {
            if (!this.is_connected) return;

            // 发送心跳请求
            const writer = this.serial_get_writer();
            const command = this.COMMANDS.HEARTBEAT + '\n';
            const buffer = new TextEncoder().encode(command);
            await writer.write(buffer);

            // 更新心跳状态
            this.last_heartbeat_time = Date.now();
        } catch (error) {
            this.show_status(`发送心跳请求失败: ${error.message}`, 'error');
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

            // 如果是滑块类型，同时更新数值显示
            if (control.type === 'range') {
                const value_display = control.nextElementSibling;
                if (value_display && value_display.className === 'slider-value') {
                    // 考虑显示偏移量
                    const param = this.config_params[key];
                    const display_value = param.displayValueOffset ? value + param.displayValueOffset : value;
                    value_display.textContent = display_value;
                }
            }
        }
    }

    /**
     * 更新UI状态
     * 包括连接按钮文本、指示灯颜色、遮罩层显示和功能按钮状态
     */
    update_ui_states() {
        const is_connected = this.is_connected;

        this.connect_toggle_button.textContent = is_connected ? '断开设备' : '连接设备';
        this.connect_toggle_button.className = is_connected ? 'secondary-btn' : 'primary-btn';

        // 更新连接状态指示灯
        if (this.connection_status_icon) {
            this.connection_status_icon.className = is_connected ? 'status-icon connected' : 'status-icon';
        }

        // 更新连接状态文本
        if (this.connection_status_text) {
            this.connection_status_text.textContent = is_connected ? '已连接' : '未连接';
        }

        // 控制遮罩层显示
        if (this.connection_overlay) {
            this.connection_overlay.style.display = is_connected ? 'none' : 'flex';
        }

        // 控制功能按钮状态
        this.reset_settings_button.disabled = !is_connected;
        this.reload_settings_button.disabled = !is_connected;
        this.save_settings_button.disabled = !is_connected;

        // 控制配置参数输入控件状态
        for (const param_key in this.config_params) {
            const input_element = document.getElementById(`config-${param_key}`);
            if (input_element) {
                input_element.disabled = !is_connected;

                // 为禁用状态添加视觉反馈
                if (!is_connected) {
                    input_element.classList.add('disabled-control');

                    // 检查是否为滑块控件，如果是，同时禁用数值显示框
                    if (input_element.type === 'range' && input_element.nextElementSibling) {
                        const value_display = input_element.nextElementSibling;
                        if (value_display.classList.contains('slider-value')) {
                            value_display.classList.add('disabled-value');
                        }
                    }
                } else {
                    input_element.classList.remove('disabled-control');

                    // 检查是否为滑块控件，如果是，同时启用数值显示框
                    if (input_element.type === 'range' && input_element.nextElementSibling) {
                        const value_display = input_element.nextElementSibling;
                        if (value_display.classList.contains('slider-value')) {
                            value_display.classList.remove('disabled-value');
                        }
                    }
                }
            }
        }
    }
    // #endregion 页面控件更新相关方法


    // #region 状态显示和弹窗相关方法
    /**
     * 显示状态消息
     * @param {string} message - 状态消息内容
     * @param {string} type - 状态类型，可选值：'info'（默认）、'success'、'error'
     */
    show_status(message, type = 'info') {
        if (typeof message === 'object') {
            console.log(message);
        } else {
            console.log(`[${type.toUpperCase()}] ${message}`);
        }
    }

    /**
     * 显示自定义弹窗
     * @param {string} title - 弹窗标题
     * @param {string} message - 弹窗消息内容
     */
    show_custom_alert(title, message) {
        this.alert_title.textContent = title;
        this.alert_message.textContent = message;
        this.custom_alert.style.display = 'flex';
    }

    /**
     * 隐藏自定义弹窗
     */
    hideCustomAlert() {
        this.custom_alert.style.display = 'none';
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
        const support_status = info.supportsSerial ? '✅ 支持' : '❌ 不支持';
        const mobile_status = info.isMobile ? '📱 移动版' : '💻 桌面版';
        const protocol_status = info.protocol === 'https:' || info.protocol === 'http:' && window.location.hostname === 'localhost' ? '✅ 符合要求' : '❌ 不符合要求';

        this.browser_info.innerHTML = `
            <strong>当前浏览器信息：</strong><br>
            - 浏览器：${info.browser} ${info.version}<br>
            - Web Serial API：${support_status}<br>
            - 设备类型：${mobile_status}<br>
            - 访问协议：${info.protocol} (${protocol_status})<br>
            - 主机名：${window.location.hostname}
        `;
    }

    /**
     * 检查浏览器是否支持Web Serial API
     * 如果不支持，显示警告并禁用连接按钮
     * @returns {boolean} 是否支持
     */
    check_browser_support() {
        const is_supported = 'serial' in navigator;
        if (!is_supported) {
            this.browser_support_alert.style.display = 'block';
            this.__display_browser_info();
            this.connect_toggle_button.disabled = true;
            return false;
        }
        this.browser_support_alert.style.display = 'none';
        return true;
    }
    // #endregion 浏览器信息相关方法
}

window.serialAssistant = null;
window.addEventListener('DOMContentLoaded', () => {
    window.serialAssistant = new SerialAssistant();
});
