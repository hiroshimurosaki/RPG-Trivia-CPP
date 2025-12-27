#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include "data/GameData.h" 
#include "systems/QuestionManager.h" 
#include "entities/Character.h"
#include <cstdlib> 
#include <cmath>
#include <ctime>   

// Variáveis Globais de Estado
GameState currentState = GameState::LanguageSelect; 
int selectedLanguage = 0; 
RpgClass currentClass;    

// --- VARIÁVEIS DE UI E ANIMAÇÃO ---
char inputBuffer[256] = ""; 
float textTimer = 0.0f;     
int visibleChars = 0;       
const float TEXT_SPEED = 0.03f; 

int lastQuestionId = -1;

// Configuração de Janela
bool isFullscreen = false;
sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
const unsigned int W_WIDTH = 1280;
const unsigned int W_HEIGHT = 720;

float currentScale = 0.0f;
float targetScale = 1.0f;

void Update(float dt) {
    currentScale += (targetScale - currentScale) * 10.0f * dt;
}

void DrawUI() {
    ImGui::SetWindowFontScale(currentScale); 
}

void ToggleFullscreen(sf::RenderWindow& window) {
    isFullscreen = !isFullscreen;
    ImGui::SFML::Shutdown();

    if (isFullscreen) {
        window.create(desktopMode, "RPG Trivia", sf::Style::Fullscreen);
    } else {
        window.create(sf::VideoMode(W_WIDTH, W_HEIGHT), "RPG Trivia", sf::Style::Titlebar | sf::Style::Close);
    }

    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);
}

void DrawGameBackground(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    float w = static_cast<float>(winSize.x);
    float h = static_cast<float>(winSize.y);

    float leftPanelW = w * 0.15f;
    float rightPanelW = w * 0.20f;
    float centerPanelW = w - leftPanelW - rightPanelW;

    // Painel Esquerdo (Amarelo)
    sf::RectangleShape pLeft(sf::Vector2f(leftPanelW, h));
    pLeft.setPosition(0, 0);
    pLeft.setFillColor(sf::Color(240, 230, 140)); 
    window.draw(pLeft);

    // Painel Direito (Vermelho)
    sf::RectangleShape pRight(sf::Vector2f(rightPanelW, h));
    pRight.setPosition(w - rightPanelW, 0);
    pRight.setFillColor(sf::Color(220, 50, 50)); 
    window.draw(pRight);

    // Painel Central (Rosa)
    sf::RectangleShape pCenter(sf::Vector2f(centerPanelW, h));
    pCenter.setPosition(leftPanelW, 0);
    pCenter.setFillColor(sf::Color(220, 130, 180)); 
    window.draw(pCenter);
}

// Funções de Tela
void ShowLanguageScreen(sf::RenderWindow& window) {
    ImGui::SetNextWindowPos(ImVec2(window.getSize().x * 0.5f, window.getSize().y * 0.5f), ImGuiCond_Always, ImVec2(0.6f, 0.6f));
    ImGui::Begin("Language", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Select Language / Selecione o Idioma");
    ImGui::Separator();
    if (ImGui::Button("Portugues (BR)", ImVec2(200, 25))) {
        selectedLanguage = 0;
        currentState = GameState::MainMenu; 
    }
    if (ImGui::Button("English", ImVec2(200, 25))) {
        selectedLanguage = 1;
        currentState = GameState::MainMenu;
    }
    ImGui::End();
}

void ShowMainMenu(sf::RenderWindow& window) {
    ImGui::SetNextWindowPos(ImVec2(window.getSize().x * 0.5f, window.getSize().y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("RPG TRIVIA - MENU");
    ImGui::Separator();
    if (ImGui::Button("JOGAR / PLAY", ImVec2(200, 40))) {
        currentState = GameState::ClassSelect; 
    }
    if (ImGui::Button("SAIR / QUIT", ImVec2(200, 40))) {
        window.close();
    }
    ImGui::End();
}

void ShowClassSelection(sf::RenderWindow& window, const std::vector<RpgClass>& classes) {
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    ImGui::SetNextWindowPos(ImVec2(window.getSize().x * 0.5f, window.getSize().y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Selecao de Classe", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Escolha sua especializacao:");
    ImGui::Separator();
    ImGui::Columns(2);
    static int selectedIdx = -1; 
    for (int i = 0; i < classes.size(); i++) {
        if (ImGui::Selectable(classes[i].name.c_str(), selectedIdx == i)) {
            selectedIdx = i;
        }
    }
    ImGui::NextColumn();
    if (selectedIdx != -1) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", classes[selectedIdx].name.c_str());
        ImGui::TextWrapped("%s", classes[selectedIdx].description.c_str());
        ImGui::Spacing();
        if (ImGui::Button("CONFIRMAR ESCOLHA", ImVec2(-1, 50))) {
            currentClass = classes[selectedIdx];
            currentState = GameState::Playing; 
        }
    } else {
        ImGui::Text("Selecione uma classe a esquerda...");
    }
    ImGui::Columns(1);
    ImGui::End();
}

bool CheckAnswer(const std::string& userInput, const std::vector<std::string>& validAnswers) {
    try {
        float userVal = std::stof(userInput); 
        for (const auto& valid : validAnswers) {
            float correctVal = std::stof(valid);
            float margin = correctVal * 0.05f; 
            if (userVal >= (correctVal - margin) && userVal <= (correctVal + margin)) {
                return true;
            }
        }
    } catch (...) {}

    for (const auto& valid : validAnswers) {
        if (userInput == valid) return true;
    }
    return false;
}

void ShowBattleUI(sf::RenderWindow& window, const Question& q, float dt, float timeTotal) {
    sf::Vector2u winSize = window.getSize();
    float w = static_cast<float>(winSize.x);
    float leftPanelW = w * 0.15f;
    float centerPanelW = w * 0.65f; 

    if (q.id != lastQuestionId) {
        lastQuestionId = q.id;
        textTimer = 0.0f;
        visibleChars = 0;
        inputBuffer[0] = '\0'; 
    }

    if (visibleChars < q.text.size()) {
        textTimer += dt;
        if (textTimer >= TEXT_SPEED) {
            visibleChars++; 
            textTimer = 0.0f;
        }
    }

    float floatingY = sin(timeTotal * 2.0f) * 5.0f;

    ImGui::SetNextWindowPos(ImVec2(leftPanelW + 20, 150 + floatingY)); 
    ImGui::SetNextWindowSize(ImVec2(centerPanelW - 40, 200));
    
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.9f, 0.7f, 0.5f, 1.0f)); 
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1)); 
    
    ImGui::Begin("Pergunta", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        ImGui::SetWindowFontScale(currentScale * 1.5f); 
        
        if (q.text.empty()) {
            ImGui::Text("Carregando...");
        } else {
            ImGui::TextColored(ImVec4(0.2f, 0.2f, 0.2f, 1), "%s - %s", q.category.c_str(), q.subtheme.c_str());
            ImGui::Separator();
            std::string textToShow = q.text.substr(0, visibleChars);
            ImGui::TextWrapped("%s", textToShow.c_str());
        }
    ImGui::End();
    ImGui::PopStyleColor(2);

    ImGui::SetNextWindowPos(ImVec2(leftPanelW + 20, 370));
    ImGui::SetNextWindowSize(ImVec2(centerPanelW - 40, 300));

    ImGui::Begin("Respostas", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowFontScale(1.3f); 
        if (q.type == QuestionType::MultipleChoice) {
            if (!q.options.empty()) {
                for (size_t i = 0; i < q.options.size(); i++) {
                    std::string label = std::string(1, 'A' + i) + ") " + q.options[i];
                    if (ImGui::Button(label.c_str(), ImVec2(-1, 60))) {
                        if (i == q.correctIndex) { /* Acertou! */ } 
                        else { /* Errou! */ }
                    }
                    ImGui::Spacing();
                }
            }
        }
        else {
            ImGui::Text("Sua Resposta:");
            bool enterPressed = ImGui::InputText("##answer", inputBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::Spacing();

            if (ImGui::Button("CONFIRMAR RESPOSTA", ImVec2(-1, 60)) || enterPressed) {
                std::string sInput(inputBuffer);
                if (CheckAnswer(sInput, q.validAnswers)) {
                     ImGui::OpenPopup("Acertou!");
                } else {
                     ImGui::OpenPopup("Errou!");
                }
            }
            if (ImGui::BeginPopup("Acertou!")) {
                ImGui::TextColored(ImVec4(0,1,0,1), "RESPOSTA CORRETA!");
                ImGui::Text("Dano causado no inimigo.");
                ImGui::EndPopup();
            }
            if (ImGui::BeginPopup("Errou!")) {
                ImGui::TextColored(ImVec4(1,0,0,1), "RESPOSTA ERRADA!");
                ImGui::Text("Voce tomou dano.");
                ImGui::EndPopup();
            }
        }
    ImGui::End();
}

// --- MAIN ---
int main() {
    sf::RenderWindow window(sf::VideoMode(W_WIDTH, W_HEIGHT), "RPG Trivia", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window)) return -1; 

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    QuestionManager qManager;
    qManager.LoadAllCategories(); 
    
    Question currentQuestion; 

    float totalTime = 0.0f;

    std::vector<RpgClass> allClasses = LoadClasses();
    sf::Clock deltaClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11) {
                ToggleFullscreen(window);
            }
        }

        sf::Time dtObj = deltaClock.restart();
        float dt = dtObj.asSeconds();
        
        totalTime += dt;

        ImGui::SFML::Update(window, dtObj);
        Update(dt); 

        window.clear(sf::Color(30, 30, 30)); 

        if (currentState == GameState::Playing) {
            DrawGameBackground(window);
        }

        switch (currentState) {
            case GameState::LanguageSelect:
                ShowLanguageScreen(window);
                break;
            case GameState::MainMenu:
                ShowMainMenu(window);
                break;
            case GameState::ClassSelect:
                ShowClassSelection(window, allClasses);
                break;
            case GameState::Playing:
                if (currentQuestion.text.empty()) {
                    currentQuestion = qManager.GetRandomQuestion();
                }
                ShowBattleUI(window, currentQuestion, dt, totalTime);
                break;
            default: break;
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}