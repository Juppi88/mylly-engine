#include "menuscene.h"
#include "game.h"
#include "ui.h"
#include "utils.h"
#include "asteroidhandler.h"
#include <mylly/core/mylly.h>
#include <mylly/mgui/widgets/button.h>
#include <mylly/resources/resources.h>

// -------------------------------------------------------------------------------------------------

MenuScene::MenuScene(void) :
	Scene()
{
}

MenuScene::~MenuScene(void)
{
	widget_destroy(m_menuPanel);
}

void MenuScene::Create(Game *game)
{
	Scene::Create(game);

	// Disable some parts of the UI and display the main menu.
	game->GetUI()->ToggleHUD(false);

	CreateMainMenu(game);
}

void MenuScene::SetupLevel(Game *game)
{
	// Select a background for the menu.
	SetBackground(3);

	// Spawn some asteroids.
	m_asteroids->SpawnInitialAsteroids(game, ASTEROID_LARGE, 3);
	m_asteroids->SpawnInitialAsteroids(game, ASTEROID_MEDIUM, 5);
}

void MenuScene::Update(Game *game)
{
	m_asteroids->Update(game);

	Scene::Update(game);
}

void MenuScene::CreateMainMenu(Game *game)
{
	// Create a container widget for the main menu and align it to the centre of the screen.
	m_menuPanel = widget_create(nullptr);
	widget_set_anchors(m_menuPanel,
		ANCHOR_MIDDLE, -500,
		ANCHOR_MIDDLE, +500,
		ANCHOR_MIDDLE, -300,
		ANCHOR_MIDDLE, +300
	);

	// Create the logo.
	widget_t *logo = Utils::CreateLabel(m_menuPanel, "ASTEROIDS", true,
		ANCHOR_MIN, 0,
		ANCHOR_MAX, 0,
		ANCHOR_MIN, 0,
		ANCHOR_MIN, 50
	);

	widget_set_text_font(logo, res_get_font("sofachrome rg it", 0));

	// Create menu buttons.
	widget_t *startGameButton = Utils::CreateButton(logo, "START",
		ANCHOR_MIDDLE, -150,
		ANCHOR_MIDDLE, 150,
		ANCHOR_MAX, 125,
		ANCHOR_MAX, 175
	);

	button_set_clicked_handler(startGameButton, OnSelectedStartGame);
	widget_set_hovered_handler(startGameButton, OnButtonHovered);
	widget_set_user_context(startGameButton, game);

	widget_t *exitGameButton = Utils::CreateButton(startGameButton, "EXIT",
		ANCHOR_MIN, 0,
		ANCHOR_MAX, 0,
		ANCHOR_MAX, 50,
		ANCHOR_MAX, 100
	);

	button_set_clicked_handler(exitGameButton, OnSelectedExitGame);
	widget_set_hovered_handler(exitGameButton, OnButtonHovered);
}

void MenuScene::OnButtonHovered(widget_t *button, bool hovered)
{
	widget_set_text_colour(button, hovered ? Utils::HOVERED_LABEL_COLOUR : Utils::LABEL_COLOUR);
}

void MenuScene::OnSelectedStartGame(widget_t *button)
{
	Game *game = (Game *)button->user_context;
	game->StartNewGame();
}

void MenuScene::OnSelectedExitGame(widget_t *button)
{
	// This will perform cleanup and exit the game immediately.
	mylly_exit();
}
