#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

int main() {

    sf::RenderWindow window(sf::VideoMode(1280, 720), "RPG Trivia - Alpha");

    window.setFramerateLimit(100);


    if (!ImGui::SFML::Init(window)) {
        return -1; // Erro ao iniciar
    }

    sf::Clock deltaClock;

    // --- LOOP PRINCIPAL DO JOGO ---
    while (window.isOpen()) {

        sf::Event event;
        while (window.pollEvent(event)) {

            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        sf::Time dt = deltaClock.restart();
        ImGui::SFML::Update(window, dt);

        ImGui::Begin("Debug do Mestre"); 
        ImGui::Text("Bem-vindo ao RPG Trivia!");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::End();
        // -------------------------------------

        window.clear(sf::Color(20, 20, 30));

        ImGui::SFML::Render(window);
        
        window.display(); 
    }


    ImGui::SFML::Shutdown();
    return 0;
}