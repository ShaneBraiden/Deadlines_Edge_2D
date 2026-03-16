#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>
#include <stdexcept>

// A single animation: a sequence of frames from one row of a spritesheet.
class Animation {
public:
    Animation() : frameDuration(0.1f) {}

    // row        = which row in the spritesheet (0-based)
    // frameCount = how many frames in that row
    // frameSize  = pixel dimensions of a single frame
    // duration   = seconds per frame
    Animation(int row, int frameCount, sf::Vector2i frameSize, float duration)
        : frameDuration(duration)
    {
        for (int i = 0; i < frameCount; ++i) {
            frames.emplace_back(
                i * frameSize.x,
                row * frameSize.y,
                frameSize.x,
                frameSize.y
            );
        }
    }

    const sf::IntRect& getFrame(int index) const { return frames[index]; }
    int getFrameCount() const { return static_cast<int>(frames.size()); }
    float getFrameDuration() const { return frameDuration; }

private:
    std::vector<sf::IntRect> frames;
    float frameDuration;
};

// Drives playback of named animations on a sprite.
class AnimationPlayer {
public:
    AnimationPlayer()
        : currentAnim(nullptr)
        , currentFrame(0)
        , elapsed(0.0f)
        , looping(true)
        , finished(false)
    {}

    void addAnimation(const std::string& name, const Animation& anim) {
        animations[name] = anim;
    }

    // Switch to a named animation. Does nothing if already playing it.
    void play(const std::string& name, bool loop = true) {
        if (currentName == name) return;

        auto it = animations.find(name);
        if (it == animations.end())
            throw std::runtime_error("AnimationPlayer: no animation named '" + name + "'");

        currentName = name;
        currentAnim = &it->second;
        currentFrame = 0;
        elapsed = 0.0f;
        looping = loop;
        finished = false;
    }

    // Advance time, update the sprite's texture rect.
    void update(float dt, sf::Sprite& sprite) {
        if (!currentAnim || currentAnim->getFrameCount() == 0) return;

        elapsed += dt;

        while (elapsed >= currentAnim->getFrameDuration()) {
            elapsed -= currentAnim->getFrameDuration();
            currentFrame++;

            if (currentFrame >= currentAnim->getFrameCount()) {
                if (looping) {
                    currentFrame = 0;
                } else {
                    currentFrame = currentAnim->getFrameCount() - 1;
                    finished = true;
                }
            }
        }

        sprite.setTextureRect(currentAnim->getFrame(currentFrame));
    }

    bool isFinished() const { return finished; }
    const std::string& getCurrentName() const { return currentName; }

private:
    std::map<std::string, Animation> animations;
    Animation* currentAnim;
    std::string currentName;
    int currentFrame;
    float elapsed;
    bool looping;
    bool finished;
};
