#include "scene.h"

// -------------------------------------------------------------------------------------------------

class MenuScene : public Scene
{
public:
	MenuScene(void);
	~MenuScene(void);

	virtual void Create(Game *game) override;
	virtual void SetupLevel(Game *game) override;
	virtual void Update(Game *game) override;

private:
	void CreateMainMenu(Game *game);
	
	static void OnButtonHovered(widget_t *button, bool hovered);
	static void OnSelectedStartGame(widget_t *button);
	static void OnSelectedExitGame(widget_t *button);

private:
	widget_t *m_menuPanel = nullptr;
};
