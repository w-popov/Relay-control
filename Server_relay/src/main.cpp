#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <map>
#include "relays.h"
#include "httplib.h"

// wPi =  16  Физический пин №26 (PA10) 
// wPi = 6    Физический pin №12 (PA07)

int main(void) 
{
    #ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN); 
    #endif

    // Инициализация портов через wiringOP
    if (!init_wiring())
    {
        return EXIT_FAILURE;
    }

    Relay k1(6, "K1", "Реле K1 включено (3.3V)", "Реле K1 отключено (0V)");
    Relay k2(16, "K2", "Реле K2 включено (3.3V)", "Реле K2 отключено (0V)");
    std::map<std::string, Relay*> relay_map = {
        {"k1", &k1},
        {"k2", &k2}
    };

    // объект сервера httplib
    httplib::Server svr;

    svr.Get("/test", [](const httplib::Request& req, httplib::Response& res)
    {
        if (req.has_param("pi"))
        {
            std::string preved = req.get_param_value("pi");
            if (!preved.compare("preved"))
            {
                res.status = 200;
                res.set_header("Access-Control-Allow-Origin", "*");
                res.set_content("medved", "text/plain; charset=utf-8");
            }
        }
        else
        {
            res.status = 400;
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_content("Error param", "text/plain; charset=utf-8");
        }
    });

    // Пин PA10 K2
    svr.Get("/k2/on", [&](const httplib::Request&, httplib::Response& res) 
    {
        k2.on();
        res.status = 200;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("Реле K2 включено (3.3V)\n", "text/plain; charset=utf-8");
    });

    svr.Get("/k2/off", [&](const httplib::Request&, httplib::Response& res) 
    {
        k2.off();
        res.status = 201;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("Реле K2 отключено (0V)\n", "text/plain; charset=utf-8");
    });

    // Пин PA07 K1
    svr.Get("/k1/on", [&](const httplib::Request&, httplib::Response& res) 
    {
        k1.on();
        res.status = 200;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("Реле K1 включено (3.3V)\n", "text/plain; charset=utf-8");
    });

    svr.Get("/k1/off", [&](const httplib::Request&, httplib::Response& res) 
    {
        k1.off();
        res.status = 201;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_content("Реле K1 отключено (0V)\n", "text/plain; charset=utf-8");
    });

    // Мигание реле
    svr.Get("/blink/:id/:ms", [&](const httplib::Request& req, httplib::Response& res) 
    {
        res.set_header("Access-Control-Allow-Origin", "*");

        std::string relay_id = req.path_params.at("id"); 
        std::string delay_str = req.path_params.at("ms");

        auto it = relay_map.find(relay_id);
        if (it == relay_map.end()) 
        {
            res.status = 404;
            res.set_content("Ошибка: Реле не найдено.\n", "text/plain; charset=utf-8");
            return;
        }

        try {
            int delay = std::stoi(delay_str);
            // Если count=0 мигание будет бесконечным пока не нажать откл
            int count = req.has_param("count") ? std::stoi(req.get_param_value("count")) : 0;

            if (delay <= 0 || count < 0) 
            {
                res.status = 400;
                res.set_content("Ошибка: Некорректные параметры!\n", "text/plain; charset=utf-8");
                return;
            }

            // Запуск потока мигания
            if (!start_blink_thread(*(it->second), count, delay)) 
            {
                res.status = 409; // HTTP 409 Conflict
                res.set_content("Реле " + it->second->pin_name + " уже работает.\n", "text/plain; charset=utf-8");
                return;
            }

            res.status = 200;
            res.set_content(it->second->pin_name + " Включено " + delay_str + "мС\n", "text/plain; charset=utf-8");
        } 
        catch (const std::exception& e) 
        {
            res.status = 400;
            res.set_content("Ошибка парсинга параметров URL.\n", "text/plain; charset=utf-8");
        }
    });


    // Изменения частоты мигания. Пример: /blink/k1/speed/100
    svr.Get("/blink/:id/speed/:ms", [&](const httplib::Request& req, httplib::Response& res) 
    {
        res.set_header("Access-Control-Allow-Origin", "*");

        std::string relay_id = req.path_params.at("id"); 
        std::string new_delay_str = req.path_params.at("ms");

        auto it = relay_map.find(relay_id);
        if (it == relay_map.end()) 
        {
            res.status = 404;
            res.set_content("Ошибка: Реле не найдено.\n", "text/plain; charset=utf-8");
            return;
        }

        try {
            int new_delay = std::stoi(new_delay_str);
            if (new_delay <= 0) 
            {
                res.status = 400;
                res.set_content("Ошибка: Задержка должна быть больше 0.\n", "text/plain; charset=utf-8");
                return;
            }

            it->second->set_delay(new_delay);

            std::string status_msg = it->second->get_thread_active() 
                ? "Частота мигания " + it->second->pin_name + " изменена на " + new_delay_str + " мс\n"
                : "Задержка мигания " + it->second->pin_name + new_delay_str + "мс)\n";

            res.status = 200;
            res.set_content(status_msg, "text/plain; charset=utf-8");
        } 
        catch (const std::exception& e) 
        {
            res.status = 400;
            res.set_content("Ошибка парсинга параметров.\n", "text/plain; charset=utf-8");
        }
    });

    // Запуск svr сервера в отдельном потоке. Порт: 5000
    std::thread server_thread([&svr]() 
    {
        if (!svr.listen("0.0.0.0", 5000)) 
        {
            std::cout << RED << "Ошибка! Не удалось занять порт 5000!\n" << NC;
        }        
    });

    // Обработка сигналов останова Ctrl+C, запуск сервера во 2 потоке.
    #ifdef SIGPIPE
    struct sigaction sa;

    sa.sa_handler = [](int) {};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    std::cout << "\n=======================================================\n";
    std::cout << GREEN << "[HTTP SERVER] ЗАПУЩЕН на http://192.168.1.6:5000" << "\n" << NC;
    std::cout << "\n=======================================================\n";

    pause();

    std::cout << "\nОстановка веб-сервера\n";
    svr.stop(); 

    // Ждать пока поток сервера не закончит работу
    if (server_thread.joinable()) 
    {
        server_thread.join();
    }
    #endif
    
    return EXIT_SUCCESS;
}

// http://192.168.1.6:5000/blink/k1/250
// http://192.168.1.6:5000//blink/k1/speed/100
