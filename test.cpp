#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

int main() {
    // Создаем HTTP сервер
    httplib::Server svr;

    // Обработчик POST запроса на /api/data
    svr.Post("/api/panel", [](const httplib::Request& req, httplib::Response& res) 
    {
        try 
        {
            // Парсим входящий JSON
            auto json_data = json::parse(req.body);
            
            // Выводим полученные данные в консоль сервера
            std::cout << "Received data:\n" << json_data.dump(4) << std::endl;
            
            // Подготавливаем ответ
            json response = {
                {"status", "success"},
                {"message", "Data received successfully"},
                {"received_data", json_data}
            };
            
            // Устанавливаем заголовок Content-Type
            //res.set_header("Content-Type", "application/json");
            
            // Отправляем ответ
            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e) 
        {
            // В случае ошибки отправляем сообщение об ошибке
            json error_response = {
                {"status", "error"},
                {"message", e.what()}
            };
            
            res.status = 400; // Bad Request
            res.set_content(error_response.dump(), "application/json");
        }
    });

    // Обработчик GET запроса на корневой путь (для проверки работы сервера)
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Server is running. Send POST request to /api/data with JSON data.", "text/plain");
    });

    // Выводим сообщение о запуске сервера
    std::cout << "Server started on http://localhost:8080" << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;

    // Запускаем сервер на порту 8080
    svr.listen("localhost", 8080);

    return 0;
}