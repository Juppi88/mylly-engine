#pragma once

#include "gamedefs.h"

// -------------------------------------------------------------------------------------------------

class UI
{
public:
	UI(void);
	~UI(void);

	void Create(void);
	void Update(void);

	void ToggleHUD(bool isVisible);

	void SetScore(uint32_t amount);
	void AddScore(uint32_t amount);

	void SetShipCount(uint32_t ships);

	void ShowLevelLabel(uint32_t level);
	void ShowLevelCompletedLabel(void);
	void ShowRespawnLabel(void);
	void ShowGameOverLabel(void);

	void HideInfoLabels(void);

private:
	bool IsUpdatingScore(void) const { return (m_scoreCounterEnds != 0); }
	bool IsFadingLevelLabel(void) const { return (m_levelFadeEnds != 0); }

	void DisplayInfoLabels(const char *levelText, const char *infoText = nullptr);
	
private:
	static constexpr float SCORE_COUNTER_DURATION = 1.0f;
	static constexpr float LEVEL_DURATION = 5.0f;

	widget_t *m_hudPanel = nullptr;
	widget_t *m_scoreLabel = nullptr;
	widget_t *m_levelLabel = nullptr;
	widget_t *m_helpLabel = nullptr;

	widget_t *m_shipSprites[3] = { nullptr, nullptr, nullptr };

	float m_scoreCounterEnds = 0;
	uint32_t m_currentScore = 0;
	uint32_t m_previousScore = 0;
	uint32_t m_targetScore = 0;

	float m_levelFadeEnds = 0;
};
