#include <iostream>
#include <utility>
#include "relays.h"

/**
 * Инициализация портов через wiringOP
 */
int init_wiring()
{
    if (wiringPiSetup() == -1) 
    {
        std::cerr << RED << "Ошибка инициализации wiringPi!" << NC << "\n";
        return 0;
    }
    return 1;
}

/**
 * Асинхронная функция мигания
 */
bool start_blink_thread(Relay& relay, int count, int delay_ms) 
{
    if (relay.get_thread_active())
    {
        return false;
    }

    relay.set_thread_active(true);
    relay.set_blink(true);
    relay.set_delay(delay_ms);

    std::thread([&relay, count, delay_ms]() 
    {
        auto blink = [&relay]()
        {
            relay.toggle();
            int current_delay = relay.get_delay();
            std::this_thread::sleep_for(std::chrono::milliseconds(current_delay));
        };

        if (count > 0)
        {
            for (int i = 0; i < count * 2; ++i) 
            {
                if (!relay.get_blink()) 
                { 
                    break; 
                }
                blink();        
            }
        }
        else
        {
            while (relay.get_blink())
            {
                blink();
            } 
        }
        
        relay.off(); 
        relay.set_thread_active(false); 

    }).detach();

    return true;
}

Relay::~Relay()
{
    if (pin != -1)
    {
        off();
    }
}

// Конструктор по умолчанию
Relay::Relay()
: pin(-1),
  pin_direction(OUTPUT),
  delay_blink(0),
  state(false),
  is_thread_active(false),
  is_blink(false),
  pin_name(""),
  msg_is_on(""),
  msg_is_off("")
{
}


Relay::Relay(int pin_num, std::string name_pin, std::string msg_on, std::string msg_off)
    : pin(pin_num),
      pin_direction(OUTPUT),
      delay_blink(500),
      state(false),
      is_thread_active(false),
      is_blink(false),
      pin_name(std::move(name_pin)),
      msg_is_on(std::move(msg_on)),
      msg_is_off(std::move(msg_off))
{
    if (pin != -1) {
        pinMode(pin, pin_direction);
        off();    
    }
}


Relay::Relay(Relay&& other) noexcept
    : pin(other.pin),
      pin_direction(other.pin_direction),
      delay_blink(other.delay_blink),
      state(other.state),
      is_thread_active(other.is_thread_active),
      is_blink(other.is_blink),
      pin_name(std::move(other.pin_name)),
      msg_is_on(std::move(other.msg_is_on)),
      msg_is_off(std::move(other.msg_is_off))
{
    other.pin = -1;
    other.pin_direction = -1;
    other.delay_blink = 0;
    other.state = false;
    other.is_blink = false;
    other.is_thread_active = false;
}


Relay& Relay::operator=(Relay&& other) noexcept
{
    if (this != &other)
    {
        if (pin != -1) 
        {
            off();
        }

        pin = other.pin;
        pin_direction = other.pin_direction;
        pin_name = std::move(other.pin_name);
        msg_is_on = std::move(other.msg_is_on);
        msg_is_off = std::move(other.msg_is_off);
        delay_blink = other.delay_blink;
        state = other.state;
        is_blink = other.is_blink;
        is_thread_active = other.is_thread_active;

        other.pin = -1;
        other.pin_direction = -1;
        other.delay_blink = 0;
        other.state = false;
        other.is_blink = false;
        other.is_thread_active = false;
    }
    return *this;
}

// Реализация логики интерфейса
// ============================

void Relay::on() 
{
    set_blink(false);
    if (pin != -1) 
    {
        digitalWrite(pin, HIGH);
        state = true;
    }
}

void Relay::off()
{
    set_blink(false);
    if (pin != -1) 
    {
        digitalWrite(pin, LOW);
        state = false;
    }
}

void Relay::toggle() 
{
    if (pin < 0)
        return;
    
    state ^= 1;
    digitalWrite(pin, state);
}

bool Relay::is_state() const 
{
    return state;
}

// Установка новой задержки мигания
void Relay::set_delay(int ms) 
{
    if (ms > 0) 
    {
        std::lock_guard<std::mutex> lock(blink_mtx);
        delay_blink = ms;
    }
}

// Получение текущей задержки мигания
int Relay::get_delay() const 
{
    std::lock_guard<std::mutex> lock(blink_mtx);
    return delay_blink;
}

// Установка флага мигания
void Relay::set_blink(bool value) 
{
    std::lock_guard<std::mutex> lock(blink_mtx);
    is_blink = value;
}

// Чтение флага мигания
bool Relay::get_blink() const 
{
    std::lock_guard<std::mutex> lock(blink_mtx);
    return is_blink;
}

// Активность потока мигания
void Relay::set_thread_active(bool value) 
{
    std::lock_guard<std::mutex> lock(blink_mtx);
    is_thread_active = value;
}

bool Relay::get_thread_active() const 
{
    std::lock_guard<std::mutex> lock(blink_mtx);
    return is_thread_active;
}

