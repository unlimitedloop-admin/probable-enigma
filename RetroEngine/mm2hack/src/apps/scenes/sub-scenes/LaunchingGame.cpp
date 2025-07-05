#include "LaunchingGame.h"

#include <string>
#include "apps/scenes/SceneID.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::scenes
{
    LaunchingGame::LaunchingGame()
    {
        utils::debug_log(L"LaunchingGame constructor called.");
        Initialize();
    }

    LaunchingGame::~LaunchingGame()
    {
        utils::debug_log(L"LaunchingGame destructor called.");
        Finalize();
    }

    void LaunchingGame::Initialize()
    {
        utils::debug_log(L"LaunchingGame initialized.");
    }

    void LaunchingGame::Finalize()
    {
        utils::debug_log(L"LaunchingGame finalized.");
    }

    void LaunchingGame::Update()
    {
        // Update logic for launching the game
    }

    SceneID LaunchingGame::GetSceneID() const
    {
        return SceneID::LaunchingGame;
    }

    std::wstring LaunchingGame::GetSceneName() const
    {
        return L"LaunchingGame";
    }
}