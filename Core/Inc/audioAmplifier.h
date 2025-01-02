#pragma once
#include <stdbool.h>

void audioAmplifierInit();
void audioAmplifierShutdown(bool shutdown);
void audioAmplifierMute(bool mute);
bool audioAmplifierIsShutdown();
bool audioAmplifierIsMuted();
