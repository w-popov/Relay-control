import { LitElement, css, html, unsafeCSS } from 'lit'
import { customElement, state,  } from 'lit/decorators.js'
import bootstrapStyles from 'bootstrap/dist/css/bootstrap.min.css?inline';

interface LogEntry {
  timestamp: string;
  message: string;
  type: 'info' | 'success' | 'danger';
}

/**
 * ДЗ № 8 (ОЭ) Попов В.Г.
 * Клиент управления Orange Pi Zero LTS H3 Alwinner по http.
 * Сервер запущен на устройсве.
 */
@customElement('relay-client-hw8')
export class RelayClient extends LitElement {
  
  static styles = css`${unsafeCSS(bootstrapStyles)}
  
  `
  @state() private _ipAddress = "192.168.1.6:5000";
  @state() private _logs: LogEntry[] = [];
  @state() private _connected = false;
  @state() private _k1Delay = 250;
  @state() private _k2Delay = 500;

  // Кнопки заблокированы, если в поле пусто или только пробелы
  private get _isIpEmpty(): boolean {
    return !this._ipAddress.trim();
  }

  private _handleInput(e: Event) {
    const input = e.target as HTMLInputElement;
    this._ipAddress = input.value;
  }

  private _addLog(message: string, type: 'success' | 'danger' | `info`) {
    const timestamp = new Date().toLocaleTimeString();
    this._logs = [{ timestamp, message, type }, ...this._logs];
  }

  // изменения полей ввода времени задержки
  private _handleDelayInput(e: Event) {
    const input = e.target as HTMLInputElement;
    const value = Number(input.value);
    const relayId = input.name;

    if (relayId === 'k1') {
      this._k1Delay = value;
    } else if (relayId === 'k2') {
      this._k2Delay = value;
    }

    if (value > 0 && !this._isIpEmpty) {
      const label = relayId === 'k1' ? 'Реле 1' : 'Реле 2';
      this._sendRequest(label, `/blink/${relayId}/speed/${value}`);
    }
  }


  /**
   * Асинхронный метод отправки запроса на Orange Pi
   * @param relayLabel Название реле для вывода в лог
   * @param endpoint Путь эндпоинта
   */
  private async _sendRequest(relayLabel: string, endpoint: string) {
    const baseUrl = this._ipAddress.startsWith('http') ? this._ipAddress : `http://${this._ipAddress}`;
    const url = `${baseUrl}${endpoint}`;

    try {
        const response = await fetch(url, { 
          method: 'GET',
          mode: 'cors' 
        });

        const text = await response.text();
        
        if (response.ok) {
          this._connected = true;
          if (response.status === 200)
            this._addLog(`${relayLabel}: ${text.trim()}`, 'success');
          else if (response.status === 201)
            this._addLog(`${relayLabel}: ${text.trim()}`, 'danger');
        } else {
          // Обработка ошибки 409 Conflict
          if (response.status === 409) {
            this._addLog(`${relayLabel}: ${text.trim()}`, 'danger');
          } else {
            this._connected = false;
            this._addLog(`Ошибка сервера ${url} [Статус: ${response.status}]`, 'danger');
          }
        }
      } catch (error) {
        this._connected = false;
        this._addLog(`Ошибка сети при запросе к ${url}!`, 'danger');
      }
  }

  /**
   * Тест подключения к серверу
   */
  private async _sendTest()
  {
    const baseUrl = this._ipAddress.startsWith('http') ? this._ipAddress : `http://${this._ipAddress}`;
    const url = `${baseUrl}/test?pi=preved`;
    try
    {
      this._addLog(`вы: preved`, 'info');

      const response = await fetch(url, {
          signal: AbortSignal.timeout(2000), 
          method: 'GET',
          mode: 'cors'
        });
        
      
      if (response.ok)
      {
        this._connected = true;
        const resp = await response.text();
        this._addLog(`сервер: ${resp}`, 'info');
      }

    } catch(error) {
      this._connected = false;
      this._addLog(`Ошибка сети при запросе к ${url}!`, 'danger');
    }
  }

  render() {
    return html`
      <div class="container my-5" style="max-width: 800px;">
        
        <span class="mb-4">(ОЭ) Попов В.Г.</span>
        <!-- Заголовок посередине -->
        <h2 class="text-center mb-4">ДЗ № 8</h2>
        <h5 class="text-center mb-4">Управление нагрузкой током 1-2 mA</h5>

        <!-- Поле ввода IP -->
        <div class="row justify-content-start mb-4">
          <div class="col-md-6 col-12 text-center">
            <label for="ipInput" class="form-label">IP-мк</label>
            <input 
              type="text" 
              class="form-control text-center" 
              id="ipInput" 
              placeholder="192.168.1.1"
              .value=${this._ipAddress}
              @input=${this._handleInput}
            />
          </div>
        </div>

        <!-- Тест сети -->
        <div class="row justify-content-start mb-4">
          <div class="col-md-6 col-12 d-flex align-items-center">
            <button class="btn btn-info flex-grow-1" @click=${() => this._sendTest()}>Тест соединения</button>
            ${this._connected ? html`
              <svg xmlns="http://w3.org" width="24" height="24" fill="#198754" class="ms-3" viewBox="0 0 16 16" style="flex-shrink: 0;">
                <path fill-rule="evenodd" d="M13.854 3.646a.5.5 0 0 1 0 .708l-7 7a.5.5 0 0 1-.708 0l-3.5-3.5a.5.5 0 1 1 .708-.708L6.5 10.293l6.646-6.647a.5.5 0 0 1 .708 0z" stroke="#198754" stroke-width="1.2"/>
              </svg>
            ` : ''}
          </div>
        </div>

        <!-- Две колонки с кнопками -->
        <div class="row g-4 mb-4">
          
          <!-- Левая колонка: Реле 1 -->
          <div class="col-md-6 col-12 text-center">
            <div class="p-3 border rounded bg-light">
              <h5 class="mb-3">Реле 1</h5>
              <div class="row g-2">
                <div class="col-6">
                  <button class="btn btn-success w-100" ?disabled=${this._isIpEmpty} @click=${() => this._sendRequest('Реле 1', '/k1/on')}>Включить</button>
                </div>
                <div class="col-6">
                  <button class="btn btn-danger w-100" ?disabled=${this._isIpEmpty} @click=${() => this._sendRequest('Реле 1', '/k1/off')}>Отключить</button>
                </div>
              </div>

              <!-- Управление миганием K1 -->
              <div class="input-group mt-2">
                <button 
                  class="btn btn-dark w-50" 
                  ?disabled=${this._isIpEmpty || this._k1Delay <= 0} 
                  @click=${() => this._sendRequest('Реле 1', `/blink/k1/${this._k1Delay}`)}
                >
                  Мигать
                </button>
                <input 
                  type="number" 
                  name="k1"
                  class="form-control text-center" 
                  placeholder="мс" 
                  min="50"
                  step="20"
                  .value=${String(this._k1Delay)}
                  @input=${this._handleDelayInput}

                  ?disabled=${this._isIpEmpty}
                />
                <span class="input-group-text">мс</span>
              </div>

            </div>
          </div>

          <!-- Правая колонка: Реле 2 -->
          <div class="col-md-6 col-12 text-center">
            <div class="p-3 border rounded bg-light">
              <h5 class="mb-3">Реле 2</h5>
              <div class="row g-2">
                <div class="col-6">
                  <button class="btn btn-success w-100" ?disabled=${this._isIpEmpty} @click=${() => this._sendRequest('Реле 2', '/k2/on')}>Включить</button>
                </div>
                <div class="col-6">
                  <button class="btn btn-danger w-100" ?disabled=${this._isIpEmpty} @click=${() => this._sendRequest('Реле 2', '/k2/off')}>Отключить</button>
                </div>
              </div>

              <!-- Управление миганием K2 -->
              <div class="input-group mt-2">
                <button 
                  class="btn btn-dark w-50" 
                  ?disabled=${this._isIpEmpty || this._k2Delay <= 0} 
                  @click=${() => this._sendRequest('Реле 2', `/blink/k2/${this._k2Delay}`)}
                >
                  Мигать
                </button>
                <input 
                  type="number" 
                  name="k2"
                  class="form-control text-center" 
                  placeholder="мс" 
                  min="50"
                  step="20"
                  .value=${String(this._k2Delay)}
                  @input=${this._handleDelayInput}
                  ?disabled=${this._isIpEmpty}
                />
                <span class="input-group-text">мс</span>
              </div>

            </div>
          </div>

        </div>

        <!-- Вывод информации (Логи) -->
        <div class="card">
          <div class="card-header d-flex justify-content-between align-items-center">
            <span>INFO:</span>
            <button class="btn btn-sm btn-secondary" ?disabled=${this._logs.length === 0} @click=${() => this._logs = []}>Очистить</button>
          </div>
          <div class="card-body bg-dark text-white p-3" style="max-height: 200px; overflow-y: auto; font-family: monospace;">
            ${this._logs.length === 0 
              ? html`<div class="text-white text-center">Нет данных</div>`
              : this._logs.map(log => {
                  let textClass = 'text-white';
                  if (log.type === 'success')
                    textClass = `text-success`;
                  else if (log.type === 'danger')
                    textClass = 'text-danger';
                  else if (log.type === 'info')
                    textClass = 'text-white';
                  return html`
                    <div class="${textClass} mb-1">
                      [${log.timestamp}] ${log.message}
                    </div>
                  `;
                })
            }
          </div>
        </div>

      </div>
    `
  }

  

}

declare global {
  interface HTMLElementTagNameMap {
    'relay-client-hw8': RelayClient
  }
}
