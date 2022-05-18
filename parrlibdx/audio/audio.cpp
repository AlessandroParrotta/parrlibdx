#include "audio.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>
#include <AL/efx.h>

#include "../debug.h"

namespace prb {
	namespace audio {
		ALCcontext* alContext;
		ALCdevice* alDevice;
		bool supportsFLOAT32 = false;

		void init() {
			//if (!alutInit(NULL, NULL)) { std::cout << "error initing alut!\n"; std::terminate(); }

			if (deb::initMsgs == deb::VERBOSE) {
				deb::log("All devices:\n");
				listAudioDevices(alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER));

				deb::log("Devices:\n");
				listAudioDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
			}

			
			//alDevice = alcOpenDevice("Generic Software"); 
			alDevice = alcOpenDevice("OpenAL Soft");
			if (!alDevice) {
				deb::log("OpenAL Soft not found\n");
				alDevice = alcOpenDevice("Generic Software");
				if (!alDevice) { deb::log("no device found\n"); checkALErrors(); std::terminate(); }
			}

			alContext = alcCreateContext(alDevice, NULL); if (!alContext) { checkALErrors(); std::terminate(); }
			if (checkALErrors()) std::terminate();

			if (!alcMakeContextCurrent(alContext)) { checkALErrors(); std::terminate(); }

			if (!alutInitWithoutContext(NULL, NULL)) { checkALErrors(); std::terminate(); }

			ALCcontext* ctx = alcGetCurrentContext();
			if (!ctx) deb::log("could not fetch current AL context\n");
			else {
				ALCdevice* dev = alcGetContextsDevice(ctx);
				if (!dev) deb::log("could not fetch AL device for current context\n");
				else {
					const ALCchar* devName = alcGetString(dev, ALC_DEVICE_SPECIFIER);
					if (!devName) deb::log("could not fetch AL device name\n");
					else if(deb::initMsgs == deb::BASIC) deb::log("AL device: ", devName, "\n");

					if (deb::initMsgs == deb::VERBOSE) {
						const ALCchar* exts = alcGetString(dev, ALC_EXTENSIONS);
						if (exts) deb::log("OpenAL supported extensions: ", exts, "\n");
						else deb::log("could not fetch OpenAL extensions\n");

						listALExtensions(alcGetString(dev, ALC_EXTENSIONS), alGetString(AL_EXTENSIONS));
					}
				}
			}
			if (checkALErrors()) std::terminate();

			if (deb::initMsgs == deb::VERBOSE)
				if (alIsExtensionPresent("AL_EXT_FLOAT32")) { deb::pr("AL_EXT_FLOAT32 supported\n"); supportsFLOAT32 = true; }
				else { deb::log("AL_EXT_FLOAT32 NOT supported\n"); }
		}

		void destroy() {
			if (!alutExit()) { deb::log("error exiting alut!\n"); checkALErrors(); }
		}

		bool checkALErrors(std::stringstream& ss) {
			bool error = false;

			ALCenum err;
			while ((err = alGetError()) != ALC_NO_ERROR) {
				ss << "AL error: ";
				switch (err) {
				case AL_INVALID_NAME: ss << "AL_INVALID_NAME"; break;
				case AL_INVALID_ENUM: ss << "AL_INVALID_ENUM"; break;
				case AL_INVALID_VALUE: ss << "AL_INVALID_VALUE"; break;
				case AL_INVALID_OPERATION: ss << "AL_INVALID_OPERATION"; break;
				case AL_OUT_OF_MEMORY: ss << "AL_OUT_OF_MEMORY"; break;
				default: ss << "unknown AL error";
				}
				ss << "\n";

				error = true;
			}

			ALenum alutErr;
			while ((alutErr = alutGetError()) != ALUT_ERROR_NO_ERROR) {

				const char* str = alutGetErrorString(alutErr);

				ss << "alut error: ";
				switch (alutErr) {
				case ALUT_ERROR_OUT_OF_MEMORY: ss << "ALUT_ERROR_OUT_OF_MEMORY"; break;
				case ALUT_ERROR_INVALID_ENUM: ss << "ALUT_ERROR_INVALID_ENUM"; break;
				case ALUT_ERROR_INVALID_VALUE: ss << "ALUT_ERROR_INVALID_VALUE"; break;
				case ALUT_ERROR_INVALID_OPERATION: ss << "ALUT_ERROR_INVALID_OPERATION"; break;
				case ALUT_ERROR_NO_CURRENT_CONTEXT: ss << "ALUT_ERROR_NO_CURRENT_CONTEXT"; break;
				case ALUT_ERROR_AL_ERROR_ON_ENTRY: ss << "ALUT_ERROR_AL_ERROR_ON_ENTRY"; break;
				case ALUT_ERROR_ALC_ERROR_ON_ENTRY: ss << "ALUT_ERROR_ALC_ERROR_ON_ENTRY"; break;
				case ALUT_ERROR_OPEN_DEVICE: ss << "ALUT_ERROR_OPEN_DEVICE"; break;
				case ALUT_ERROR_CLOSE_DEVICE: ss << "ALUT_ERROR_CLOSE_DEVICE"; break;
				case ALUT_ERROR_CREATE_CONTEXT: ss << "ALUT_ERROR_CREATE_CONTEXT"; break;
				case ALUT_ERROR_MAKE_CONTEXT_CURRENT: ss << "ALUT_ERROR_MAKE_CONTEXT_CURRENT"; break;
				case ALUT_ERROR_DESTROY_CONTEXT: ss << "ALUT_ERROR_DESTROY_CONTEXT"; break;
				case ALUT_ERROR_GEN_BUFFERS: ss << "ALUT_ERROR_GEN_BUFFERS"; break;
				case ALUT_ERROR_BUFFER_DATA: ss << "ALUT_ERROR_BUFFER_DATA"; break;
				case ALUT_ERROR_IO_ERROR: ss << "ALUT_ERROR_IO_ERROR"; break;
				case ALUT_ERROR_UNSUPPORTED_FILE_TYPE: ss << "ALUT_ERROR_UNSUPPORTED_FILE_TYPE"; break;
				case ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE: ss << "ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE"; break;
				case ALUT_ERROR_CORRUPT_OR_TRUNCATED_DATA: ss << "ALUT_ERROR_CORRUPT_OR_TRUNCATED_DATA"; break;
				default: ss << "unknown alut error";
				}

				if (str) {
					ss << ": " << str;
				}

				ss << "\n";

				error = true;
			}

			if (error) deb::msberr(ss.str());

			return error;
		}
		bool checkALErrors() { std::stringstream ss; bool res = checkALErrors(ss); deb::log(ss.str(), "\n"); return res; }

		void listAudioDevices(const ALCchar* devices) {
			const ALCchar* device = devices, * next = devices + 1;
			size_t len = 0;

			deb::log("Devices list:\n");
			deb::log("----------\n");
			while (device && *device != '\0' && next && *next != '\0') {
				deb::log(device, "\n");
				len = strlen(device);
				device += (len + 1);
				next += (len + 2);
			}
			deb::log("----------\n");
			//deb::msbinfo();
		}

		void listALExtensions(const ALCchar* alcexts, const ALchar* alexts) {
			if (alcexts) {
				deb::log("supported ALC extensions: \n");

				//if(exts[0] == '\0' && exts[1] == '\0') deb::ss << "ALL ZERO!\n";

				std::string tstr = "";
				int i = 0;
				while (alcexts[i] != '\0' && alcexts[i + 1] != '\0') {
					if (alcexts[i] == ' ') { deb::log("\t", tstr.c_str(), "\n"); tstr = ""; }
					else { tstr += alcexts[i]; }
					i++;
				}
			}
			else { deb::log("could not fetch ALC extensions\n"); return; }

			if (alexts) {
				deb::log("supported AL extensions: \n");

				std::string tstr = "";
				int i = 0;
				while (alexts[i] != '\0' && alexts[i + 1] != '\0') {
					if (alexts[i] == ' ') { deb::log("\t", tstr.c_str(), "\n"); tstr = ""; }
					else { tstr += alexts[i]; }

					i++;
				}
			}
			else { deb::log("could not fetch AL extensions\n"); return; }
		}

		std::string ALformatToStr(ALenum alFormat) {
			switch (alFormat) {
			case AL_FORMAT_MONO8:			return "AL_FORMAT_MONO8";
			case AL_FORMAT_MONO16:			return "AL_FORMAT_MONO16";
			case AL_FORMAT_STEREO8:			return "AL_FORMAT_STEREO8";
			case AL_FORMAT_STEREO16:		return "AL_FORMAT_STEREO16";
			case AL_FORMAT_MONO_FLOAT32:	return "AL_FORMAT_MONO_FLOAT32";
			case AL_FORMAT_STEREO_FLOAT32:	return "AL_FORMAT_STEREO_FLOAT32";
			}
			return "unknown";
		}
	}
}

//void listAudioDevices(const ALCchar* devices) {
//	const ALCchar* device = devices, * next = devices + 1;
//	size_t len = 0;
//
//	fprintf(stdout, "Devices list:\n");
//	fprintf(stdout, "----------\n");
//	while (device && *device != '\0' && next && *next != '\0') {
//		fprintf(stdout, "%s\n", device);
//		len = strlen(device);
//		device += (len + 1);
//		next += (len + 2);
//	}
//	fprintf(stdout, "----------\n");
//
//	const ALCchar* device = devices, * next = devices + 1;
//	size_t len = 0;
//
//
//}
////listAudioDevices(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
