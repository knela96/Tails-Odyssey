#include "p2Defs.h"
#include "p2Log.h"
#include "j1Audio.h"
#include "p2List.h"
#include "j1App.h"

#include "SDL/include/SDL.h"
#include "SDL_mixer\include\SDL_mixer.h"
#pragma comment( lib, "SDL_mixer/libx86/SDL2_mixer.lib" )

j1Audio::j1Audio() : j1Module()
{
	music = NULL;
	name.create("audio");
}

// Destructor
j1Audio::~j1Audio()
{}

// Called before render is available
bool j1Audio::Awake(pugi::xml_node& config)
{

	music_path.create(config.child("music").child("folder").child_value());
	fx_path.create(config.child("fx").child("folder").child_value());

	v_music = config.child("music").attribute("volume").as_float();
	v_fx = config.child("fx").attribute("volume").as_float();

	Load(App->GetSaveData().child("audio"));


	LOG("Loading Audio Mixer");
	bool ret = true;
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		LOG("SDL_INIT_AUDIO could not initialize! SDL_Error: %s\n", SDL_GetError());
		active = false;
		ret = true;
	}

	// load support for the JPG and PNG image formats
	int flags = MIX_INIT_OGG;
	int init = Mix_Init(flags);

	if((init & flags) != flags)
	{
		LOG("Could not initialize Mixer lib. Mix_Init: %s", Mix_GetError());
		active = false;
		ret = true;
	}

	//Initialize SDL_mixer
	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		LOG("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		active = false;
		ret = true;
	}

	return ret;
}

// Called before quitting
bool j1Audio::CleanUp()
{
	if(!active)
		return true;

	LOG("Freeing sound FX, closing Mixer and Audio subsystem");

	if(music != NULL)
	{
		Mix_FreeMusic(music);
	}

	p2List_item<Mix_Chunk*>* item;
	for(item = fx.start; item != NULL; item = item->next)
		Mix_FreeChunk(item->data);

	fx.clear();

	Mix_CloseAudio();
	Mix_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);

	return true;
}

// Play a music file
bool j1Audio::PlayMusic(const char* path, float fade_time)
{
	bool ret = true;

	if(!active)
		return false;

	if(music != NULL)
	{
		if(fade_time > 0.0f)
		{
			Mix_FadeOutMusic(int(fade_time * 1000.0f));
		}
		else
		{
			Mix_HaltMusic();
		}

		// this call blocks until fade out is done
		Mix_FreeMusic(music);
	}

	p2SString tmp("%s%s", music_path.GetString(), path);

	music = Mix_LoadMUS(tmp.GetString());

	ChangeMusicVolume();

	if(music == NULL)
	{
		LOG("Cannot load music %s. Mix_GetError(): %s\n", path, Mix_GetError());
		ret = false;
	}
	else
	{
		if(fade_time > 0.0f)
		{
			if(Mix_FadeInMusic(music, -1, (int) (fade_time * 1000.0f)) < 0)
			{
				LOG("Cannot fade in music %s. Mix_GetError(): %s", path, Mix_GetError());
				ret = false;
			}
		}
		else
		{
			if(Mix_PlayMusic(music, -1) < 0)
			{
				LOG("Cannot play in music %s. Mix_GetError(): %s", path, Mix_GetError());
				ret = false;
			}
		}
	}

	//Mix_VolumeMusic(1.0f);
	LOG("Successfully playing %s", path);
	return ret;
}

// Load WAV
unsigned int j1Audio::LoadFx(const char* path)
{
	unsigned int ret = 0;

	if(!active)
		return 0;

	p2SString tmp("%s%s", fx_path.GetString(), path);

	Mix_Chunk* chunk = Mix_LoadWAV(tmp.GetString());

	if(chunk != NULL)
		ChangeFxVolume(chunk);

	if(chunk == NULL)
	{
		LOG("Cannot load wav %s. Mix_GetError(): %s", path, Mix_GetError());
	}
	else
	{
		fx.add(chunk);
		ret = fx.count();
	}

	return ret;
}

// Play WAV
bool j1Audio::PlayFx(unsigned int id, int repeat)
{
	bool ret = false;

	if(!active)
		return false;

	if(id > 0 && id <= fx.count())
	{
		ChangeFxVolume(fx[id - 1]);
		Mix_PlayChannel(-1, fx[id - 1], repeat);
	}

	return ret;
}

void j1Audio::StopMusic() {
	Mix_HaltMusic();
}

void j1Audio::StopFx() {
	Mix_HaltChannel(-1);
}

void j1Audio::ChangeMusicVolume() {
	Mix_VolumeMusic(MIX_MAX_VOLUME - (MIX_MAX_VOLUME - (int)(v_music * MIX_MAX_VOLUME)));
}

void j1Audio::ChangeFxVolume(Mix_Chunk* fx) {
	Mix_VolumeChunk(fx,MIX_MAX_VOLUME - (MIX_MAX_VOLUME - (int)(v_fx * MIX_MAX_VOLUME)));
}

void j1Audio::UnloadFx() {
	p2List_item<Mix_Chunk*>* item;
	for (item = fx.start; item != NULL; item = item->next)
		Mix_FreeChunk(item->data);
	fx.clear();
}

// Load Game State
bool j1Audio::Load(pugi::xml_node& data)
{
	if (data != nullptr) {
		v_music = data.child("v_music").attribute("volume").as_float();
		v_fx = data.child("v_fx").attribute("volume").as_float();
	}

	return true;
}

// Save Game State
bool j1Audio::Save(pugi::xml_node& data) const
{
	pugi::xml_node audio = data;

	audio.append_child("v_music").append_attribute("volume") = v_music;
	audio.append_child("v_fx").append_attribute("volume") = v_fx;

	return true;
}