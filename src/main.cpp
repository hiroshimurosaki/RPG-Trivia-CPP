#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include "data/GameData.h" 
#include "systems/QuestionManager.h" 
#include "entities/Character.h"
#include <cstdlib> 
#include <cmath>
#include <ctime>   
#include <iostream>

// --- ESTADO DO JOGO  ---
struct BattleState {
    std::vector<Character> allies;   // Time da Esquerda
    std::vector<Character> enemies;  // Time da Direita
    
    // Quem está jogando agora? (0 = Player 1, etc...)
    int turnIndex = 0; 
    bool isPlayerTurn = true; 
};

BattleState battleData; // Variável global do estado

// Variáveis Globais de Estado
GameState currentState = GameState::LanguageSelect; 
int selectedLanguage = 0; 
RpgClass currentClass;    

// Variáveis de UI e Animação
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

// Variáveis Globais de Largura (para o layout)
float panelLeftW, panelCenterW, panelRightW;

float currentScale = 0.0f;
float targetScale = 1.0f;

// --- FUNÇÕES AUXILIARES ---

void Update(float dt) {
    currentScale += (targetScale - currentScale) * 10.0f * dt;
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
    (void)ImGui::SFML::Init(window);
}

// 1. Desenha o Fundo (Layout 25-50-25)
void DrawGameBackground(sf::RenderWindow& window) {
    sf::Vector2u winSize = window.getSize();
    float w = static_cast<float>(winSize.x);
    float h = static_cast<float>(winSize.y);

    panelLeftW = w * 0.25f;
    panelRightW = w * 0.25f;
    panelCenterW = w * 0.50f; 

    // Esquerda (Amarelo)
    sf::RectangleShape pLeft(sf::Vector2f(panelLeftW, h));
    pLeft.setPosition(0, 0);
    pLeft.setFillColor(sf::Color(240, 230, 140)); 
    window.draw(pLeft);

    // Direita (Vermelho)
    sf::RectangleShape pRight(sf::Vector2f(panelRightW, h));
    pRight.setPosition(w - panelRightW, 0);
    pRight.setFillColor(sf::Color(220, 50, 50)); 
    window.draw(pRight);

    // Centro (Rosa)
    sf::RectangleShape pCenter(sf::Vector2f(panelCenterW, h));
    pCenter.setPosition(panelLeftW, 0);
    pCenter.setFillColor(sf::Color(220, 130, 180)); 
    window.draw(pCenter);
}

// 2. Desenha HUD sobre a cabeça (Vida e Buffs)
void DrawUnitHUD(Character& ch, sf::Vector2f charPos, float scale) {
    // Posição da HUD: Acima da cabeça do boneco
    ImVec2 hudPos = ImVec2(charPos.x - 60, charPos.y - (40 * scale) - 50); 
    
    ImGui::SetNextWindowPos(hudPos);
    ImGui::Begin(std::string("##HUD_" + ch.name).c_str(), nullptr, 
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize);
    
    // Nome
    ImGui::SetCursorPosX(30); 
    ImGui::Text("%s", ch.name.c_str());

    // Barra de HP
    float hpPercent = (float)ch.currentHp / (float)ch.maxHp;
    ImVec4 hpColor = (hpPercent < 0.3f) ? ImVec4(1,0,0,1) : ImVec4(0,1,0,1);
    
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, hpColor);
    char overlay[32];
    sprintf(overlay, "%d/%d", ch.currentHp, ch.maxHp);
    ImGui::ProgressBar(hpPercent, ImVec2(120, 15), overlay);
    ImGui::PopStyleColor();

    // Buffs e Debuffs
    if (!ch.buffs.empty()) {
        for (const auto& buff : ch.buffs) {
            ImVec4 buffColor = buff.isDebuff ? ImVec4(1, 0, 0, 1) : ImVec4(0, 1, 0, 1);
            ImGui::PushStyleColor(ImGuiCol_Button, buffColor);
            ImGui::Button("##b", ImVec2(10, 10)); // Ícone
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s (%d turnos)", buff.name.c_str(), buff.turnsRemaining);
            ImGui::PopStyleColor();
            ImGui::SameLine();
        }
    }
    ImGui::End();
}

// 3. Renderiza Time (Distribuição Vertical)
void RenderTeam(sf::RenderWindow& window, std::vector<Character>& team, float panelX, float panelW, float winHeight) {
    if (team.empty()) return;

    // Calcula espaço vertical para cada um
    float gap = winHeight / (team.size() + 1);
    
    // Escala dinâmica
    float scale = (team.size() > 3) ? 1.5f : 2.5f; 

    for (size_t i = 0; i < team.size(); i++) {
        float posX = panelX + (panelW / 2); // Centro do painel
        float posY = gap * (i + 1);         // Espaçamento igual

        // 1. Desenha Boneco e Itens (SFML)
        team[i].Draw(window, sf::Vector2f(posX, posY), scale);

        // 2. Desenha UI (ImGui)
        if (currentState == GameState::Playing) {
             DrawUnitHUD(team[i], sf::Vector2f(posX, posY), scale);
        }
    }
}

// --- FUNÇÕES DE TELAS (MENUS) ---

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

// 4. Interface de Batalha (Perguntas)
void ShowBattleUI(sf::RenderWindow& window, const Question& q, float dt, float timeTotal) {
    sf::Vector2u winSize = window.getSize();
    float w = static_cast<float>(winSize.x);
    
    // ATUALIZADO PARA NOVO LAYOUT
    float leftPanelW = w * 0.25f; 
    float centerPanelW = w * 0.50f; 

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
                        if (i == q.correctIndex) { 
                             ImGui::OpenPopup("Acertou!");
                             // Exemplo: Causa dano no primeiro inimigo
                             if (!battleData.enemies.empty()) battleData.enemies[0].TakeDamage(25);
                        } 
                        else { 
                             ImGui::OpenPopup("Errou!");
                             // Exemplo: Causa dano no primeiro aliado
                             if (!battleData.allies.empty()) battleData.allies[0].TakeDamage(15);
                        }
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
                     if (!battleData.enemies.empty()) battleData.enemies[0].TakeDamage(25);
                } else {
                     ImGui::OpenPopup("Errou!");
                     if (!battleData.allies.empty()) battleData.allies[0].TakeDamage(15);
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

    // --- SETUP VISUAL DOS TIMES (TESTE) ---
    sf::Texture defaultTex;
    defaultTex.create(64, 64);
    sf::Uint8* pixels = new sf::Uint8[64 * 64 * 4];
    for(int i=0; i<64*64*4; i++) pixels[i] = 255; 
    defaultTex.update(pixels);

    sf::Texture weaponTex;
    weaponTex.create(10, 40);
    sf::Uint8* wPixels = new sf::Uint8[10 * 40 * 4];
    for(int i=0; i<10*40*4; i++) {
         wPixels[i] = 0; wPixels[i+1] = 0; wPixels[i+2] = 0; wPixels[i+3] = 255; 
    }
    weaponTex.update(wPixels);

    // Aliado 1
    Character p1;
    p1.Setup("Engenheiro", 100, sf::Color::Blue);
    p1.EquipItem(EquipSlot::Weapon, weaponTex);
    p1.AddBuff("Escudo", BuffType::Shield, 3, false);
    p1.bodySprite.setTexture(defaultTex); // Define textura
    p1.bodySprite.setOrigin(32, 32);      // Centraliza
    battleData.allies.push_back(p1);

    // Aliado 2
    Character p2;
    p2.Setup("Biologo", 80, sf::Color::Cyan);
    p2.bodySprite.setTexture(defaultTex);
    p2.bodySprite.setOrigin(32, 32);
    battleData.allies.push_back(p2);

    // Inimigos (3 Goblins)
    for(int i=0; i<3; i++) {
        Character en;
        en.Setup("Goblin " + std::to_string(i+1), 50, sf::Color::Green);
        en.bodySprite.setTexture(defaultTex);
        en.bodySprite.setOrigin(32, 32);
        en.AddBuff("Veneno", BuffType::Poison, 2, true);
        battleData.enemies.push_back(en);
    }
    // --------------------------------------

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
            // 1. Desenha Fundo
            DrawGameBackground(window);
            
            // 2. Renderiza Times (Dinamicamente)
            RenderTeam(window, battleData.allies, 0, panelLeftW, (float)W_HEIGHT);
            RenderTeam(window, battleData.enemies, W_WIDTH - panelRightW, panelRightW, (float)W_HEIGHT);
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