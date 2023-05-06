#pragma once
#include "technique.h"

class PSUpdateTechnique : public Technique
{
public:
    PSUpdateTechnique();

    virtual bool Init();

    void SetParticleLifetime(float Lifetime);

    void SetDeltaTimeMillis(float DeltaTimeMillis);

    void SetTime(int Time);

    void SetRandomTextureUnit(unsigned int TextureUnit);

    void SetLauncherLifetime(float Lifetime);

    void SetShellLifetime(float Lifetime);

    void SetSecondaryShellLifetime(float Lifetime);

private:
    GLuint m_deltaTimeMillisLocation;
    GLuint m_randomTextureLocation;
    GLuint m_timeLocation;
    GLuint m_launcherLifetimeLocation;
    GLuint m_shellLifetimeLocation;
    GLuint m_secondaryShellLifetimeLocation;
};