#pragma once
#include <string_view>
#include <fstream>
#include "data.h"

class MidiFile
{
	std::uint32_t m_delta;
	std::ofstream m_file;
	std::streampos m_trackPos;
	bool Init(std::string_view fileName, const std::uint32_t bpm);
	void WriteVarLength(int value);
	void StartFile();
	void CreateTrackChunk();
	void CalculateTrackSize();

public:
	enum class Events : std::uint8_t
	{
		NoteOn = 0x90,
		NoteOff = 0x80
	};

	enum class ControlChange : std::uint8_t
	{
		ControlMessage = 0xB0,
		AllNotesOff = 0x7B
	};

	enum class MetaEvents : std::uint8_t
	{
		StartMeta = 0xFF,
		SetTempoEvent = 0x51,
		SetBPM = 0x03,
		EndTrack = 0x2F
	};	

	MidiFile() = default;
	explicit MidiFile(std::string_view fileName, const std::uint32_t bpm, const std::uint32_t delta) : m_delta(delta)
	{
		if (!Init(fileName, bpm))
		{
			throw std::invalid_argument("Invalid argument: initialization failed");
		}
	};
	
	void EndFile();
	void SetTempo(const std::uint32_t bpm);
	void NoteOn(const std::uint32_t delta, const Notes note, const std::uint8_t velocity);
	void NoteOff(const std::uint32_t delta, const Notes note);

	template <class... Ts>
	void WriteChord(const std::uint32_t endDelta, const Ts& ...args);
	template <typename... Ts>
	void WriteEvent(const std::uint32_t delta, const std::uint8_t Event, const Ts& ...args);
	template <class... Ts>
	void Write(Ts&&... args);
	template <class... Ts>
	void Put(Ts&&... args);
	template <class... Ts>
	void ByteSwap(Ts&... args)
	{
		((args = std::byteswap(args)), ...);
	}

};

template<class ...Ts>
inline void MidiFile::WriteChord(const std::uint32_t endDelta, const Ts & ...args)
{
	((this->NoteOn(0, args, 127)), ...);
	int counter = 0;
	(([this, &counter, &args, endDelta]() {
		if (counter == 0) this->NoteOff(endDelta, args);
		else this->NoteOff(0, args);
		counter += 1;
		}()), ...);
}

template<typename ...Ts>
inline void MidiFile::WriteEvent(std::uint32_t delta, std::uint8_t Event, const Ts& ...args)
{
	WriteVarLength(delta);
	Write(Event);
	((Write(args)), ...);
}

template<class... Ts>
inline void MidiFile::Write(Ts&&... args)
{
	((m_file.write(reinterpret_cast<const char*>(&args), sizeof(args))), ...);
}

template<class ...Ts>
inline void MidiFile::Put(Ts&& ...args)
{
	((m_file.put(static_cast<char>(args))), ...);
}
