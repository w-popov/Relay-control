#ifndef RELAYS_H
#define RELAYS_H

#include <wiringPi.h>
#include <mutex>
#include <thread>

#define NC      "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"

/**
 * Инициализация портов через wiringOP
 */
int init_wiring();


/**
 * Интерфейс дискретного устройства
 */
class IDiscrette
{
public:
    virtual ~IDiscrette() = default;

    // Включить устройство
    virtual void on() = 0;

    // Выключить устройство
    virtual void off() = 0;

    // Переключить состояние
    virtual void toggle() = 0;

    // Получить текущее состояние
    virtual bool is_state() const = 0;
};

/**
 * Реле
 */
class Relay : public IDiscrette
{
private:
    int pin;                    // Номер wPi пина
    int pin_direction;          // Направление пина (вход/выход)
    int delay_blink;            // Задержка мигания
    bool state;                 // Текущее состояние (true - включено)

    bool is_thread_active;        // Запущен ли сейчас поток мигания
    mutable std::mutex blink_mtx; // Мьютекс для защиты флага is_blink
    
public:
    bool is_blink;              // Мигать?
    std::string pin_name;       // Имя 
    std::string msg_is_on;      // Сообщение когда пин включен
    std::string msg_is_off;     // Сообщение когда пин отключен


    ~Relay() override;

    // Запрет копирования
    Relay(const Relay& other) = delete;
    Relay& operator=(const Relay& other) = delete;

    // Конструктор и оператор перемещения
    Relay(Relay&& other) noexcept;
    Relay& operator = (Relay&& other) noexcept;

    // Конструктор по умолчанию
    Relay();

    // Конструктор 2
    Relay(int pin_num, std::string name_pin, std::string msg_on, std::string msg_off);

    // Интерфейсные методы
    void on() override;
    void off() override;
    void toggle() override;
    bool is_state() const override;

    // Методы класса
    void set_delay(int ms);
    int get_delay() const;
    void set_blink(bool value);
    bool get_blink() const;
    void set_thread_active(bool value);
    bool get_thread_active() const;
};

/**
 * Асинхронная функция мигания
 */
bool start_blink_thread(Relay& relay, int count, int delay_ms);

#endif