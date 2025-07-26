#include "pch.h"

#include "LaunchingGame.h"

#include "apps/parameters/Parameters.h"
#include "apps/scenes/SceneID.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::scenes
{
    LaunchingGame::LaunchingGame()
    {
        utils::debug_log(L"LaunchingGame constructor called.");
    }

    LaunchingGame::~LaunchingGame()
    {
        utils::debug_log(L"LaunchingGame destructor called.");
        Finalize();
    }

    void LaunchingGame::Initialize(const parameters::Parameters& params)
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

    void LaunchingGame::Draw()
    {
        // Drawing logic for the launching game scene
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