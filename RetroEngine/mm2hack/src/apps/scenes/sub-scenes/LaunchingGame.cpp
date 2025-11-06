#include "pch.h"

#include "LaunchingGame.h"

#include "apps/resources/parameters/Parameters.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::scenes
{
    LaunchingGame::LaunchingGame(SceneChangeMediator* mediator)
        : _mediator(mediator)
    {
        utils::debug_log(kClassName + L" constructor called.");
    }

    LaunchingGame::~LaunchingGame()
    {
        utils::debug_log(kClassName + L" destructor called.");
        finalize_();
    }

    void LaunchingGame::Initialize(const Parameters& params)
    {
        utils::debug_log(kClassName + L" initialized.");

        if (auto subsequent = params.Get<SceneID>(L"Subsequent"); subsequent.has_value())
        {
            _subsequentScene = subsequent.value();
        }
        else
        {
            _subsequentScene = SceneID::None;
        }
    }

    void LaunchingGame::finalize_()
    {
        utils::debug_log(kClassName + L" finalized.");
    }

    void LaunchingGame::Update()
    {
        // Update logic for launching the game

        // Check out what the asset files used in each scene are.
        // After that, transition to the subsequent scene.
        _mediator->RequestChange(_subsequentScene);
    }
}