#include "scene.h"

// -------------------------------------------------------------------------------------------------

class GameScene : public Scene
{
public:
	GameScene(void);
	~GameScene(void);

	virtual void Create(Game *game) override;
	virtual void SetupLevel(Game *game) override;
	virtual void Update(Game *game) override;

private:
	
private:
	Ship *m_ship = nullptr;
};
