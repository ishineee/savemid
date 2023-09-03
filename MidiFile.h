/*
 *	MidiFile.h made by ishineee
 *  You are allowed to use this library in your project, but you need to give credit.
 */

#pragma once
#include <string_view>
#include <fstream>
#include <vector>
#include <utility>
#include <bit>
#include <array>

enum class Notes : std::uint8_t
{
	INVALID_NOTE = 11,
	C,
	C_SHARP,
	D,
	D_SHARP,
	E,
	F,
	F_SHARP,
	G,
	G_SHARP,
	A,
	A_SHARP,
	B,
	C2,
	C2_SHARP,
	D2,
	D2_SHARP,
	E2,
	F2,
	F2_SHARP,
	G2,
	G2_SHARP,
	A2,
	A2_SHARP,
	B2,
	C3,
	C3_SHARP,
	D3,
	D3_SHARP,
	E3,
	F3,
	F3_SHARP,
	G3,
	G3_SHARP,
	A3,
	A3_SHARP,
	B3,
	C4,
	C4_SHARP,
	D4,
	D4_SHARP,
	E4,
	F4,
	F4_SHARP,
	G4,
	G4_SHARP,
	A4,
	A4_SHARP,
	B4,
	C5,
	C5_SHARP,
	D5,
	D5_SHARP,
	E5,
	F5,
	F5_SHARP,
	G5,
	G5_SHARP,
	A5,
	A5_SHARP,
	B5,
	C6,
	C6_SHARP,
	D6,
	D6_SHARP,
	E6,
	F6,
	F6_SHARP,
	G6,
	G6_SHARP,
	A6,
	A6_SHARP,
	B6
};

struct Note
{
	std::uint32_t deltaStart;
	std::uint32_t deltaEnd;
	Notes note;
	std::uint8_t velocity;

	Note(
		const std::uint32_t delta_start,
		const std::uint32_t delta_end,
		const Notes note,
		const std::uint8_t velocity
	) : deltaStart(delta_start), deltaEnd(delta_end), note(note), velocity(velocity) {}
};

struct NoteGroup
{
	std::pair<bool, int> ifOverlap;
	std::vector<Note> notes;
};

class MidiFile
{
	std::uint32_t m_ticksPerQuarter;
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
	explicit MidiFile(std::string_view fileName, const std::uint32_t bpm, const std::uint32_t delta) : m_ticksPerQuarter(delta)
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

	template <typename... Ts>
	void WriteNotes(const Ts&& ...args);
	template <typename... Ts>
	void WriteChord(const std::uint32_t endDelta, const Ts&& ...args);
	template <typename... Ts>
	void WriteMulNotes(const Ts& ...args);
	template <typename... Ts>
	void WriteEvent(const std::uint32_t delta, const std::uint8_t Event, const Ts& ...args);
	template <typename... Ts>
	void Write(Ts&&... args);
	template <typename... Ts>
	void Put(Ts&&... args);
	template <typename... Ts>
	void ByteSwap(Ts&... args);
};

bool MidiFile::Init(std::string_view fileName, const std::uint32_t bpm)
{
	std::ofstream file;
	file.open(fileName.data(), std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}
	m_file = std::move(file);
	StartFile();
	CreateTrackChunk();
	SetTempo(bpm);
	return true;
}

void MidiFile::WriteVarLength(int value)
{
	std::array<int, 4> buffer{};
	int i = 0;
	do {
		buffer[i] = value & 0x7F;
		value >>= 7;
		i++;
	} while (value > 0);

	for (int j = i - 1; j >= 0; j--) {
		if (j > 0) {
			buffer[j] |= 0x80;
		}
		m_file.put(static_cast<char>(buffer[j]));
	}
}

// Writes a header chunk into the file.
void MidiFile::StartFile()
{
	std::uint8_t division{ static_cast<std::uint8_t>(m_ticksPerQuarter) };
	ByteSwap(division);
	m_file.write("MThd", 4);
	Put(0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00);
	Write(division);
}

void MidiFile::CalculateTrackSize()
{
	const auto val = static_cast<std::uint32_t>(m_trackPos);
	const auto val2 = static_cast<std::uint32_t>(m_file.tellp());
	std::uint32_t size = val2 - val - 8;
	m_file.seekp(m_trackPos);
	ByteSwap(size);
	Write(size);
}

// Writes the SetTempo meta event into the file, 
// does a calculation and puts the result as a 24-bit integer into the file
void MidiFile::SetTempo(const std::uint32_t bpm)
{
	using enum MidiFile::MetaEvents;
	Put(0x00, StartMeta, SetTempoEvent, SetBPM);
	std::uint32_t bpmcalc = 60000000 / bpm;
	Put((bpmcalc >> 16) & 0xFF, (bpmcalc >> 8) & 0xFF, bpmcalc & 0xFF);
}

// Writes the EndTrack event into the file and calculates the track chunk size.
void MidiFile::EndFile()
{
	Put(0x00, MetaEvents::StartMeta, MetaEvents::EndTrack, 0x00);
	CalculateTrackSize();
}

// Writes the NoteOn event into the file using WriteEvent function.
void MidiFile::NoteOn(const std::uint32_t delta, const Notes note, const std::uint8_t velocity)
{
	WriteEvent(
		delta,
		static_cast<std::uint8_t>(Events::NoteOn),
		static_cast<std::uint8_t>(note),
		velocity
	);
}

// Writes the NoteOff event into the file using WriteEvent function.
void MidiFile::NoteOff(const std::uint32_t delta, const Notes note)
{
	WriteEvent(
		delta,
		static_cast<std::uint8_t>(Events::NoteOff),
		static_cast<std::uint8_t>(note),
		static_cast<std::uint8_t>(0x00)
	);
}

void MidiFile::CreateTrackChunk()
{
	m_file.write("MTrk", 4);
	std::streampos pos = m_file.tellp();
	Put(0x00, 0x00, 0x00, 0x00);
	m_trackPos = std::move(pos);
}

// Writes a single note at a time. Takes the Note struct.
template<typename ...Ts>
inline void MidiFile::WriteNotes(const Ts && ...args)
{
	static_assert((... && std::is_same_v<Ts, Note>));

	((NoteOn(args.deltaStart, args.note, args.velocity), NoteOff(args.deltaEnd, args.note)), ...);
}

// Writes a chord at a time. Takes the variadic arguments as Notes type.
template<typename ...Ts>
inline void MidiFile::WriteChord(const std::uint32_t endDelta, const Ts&& ...args)
{
	static_assert((... && std::is_same_v<Ts, Notes>));

	((this->NoteOn(0, args, 100)), ...);
	int counter = 0;
	(([this, &counter, &args, endDelta]() {
		this->NoteOff(counter == 0 ? endDelta : 0, args);
		counter += 1;
		}()), ...);
}

template<typename ...Ts>
inline void MidiFile::WriteMulNotes(const Ts & ...args)
{
	static_assert((... && std::is_same_v<Ts, NoteGroup>));

	const auto forloop = [this](const std::vector<Note>& notes, const std::pair<bool, int>& ifOverlap)
		{
			int curTick = 0;
			for (const Note& note : notes) { curTick += note.deltaStart; }
			int counter = 0;
			for (const Note& note : notes) { this->NoteOn(note.deltaStart, note.note, note.velocity); }
			for (const Note& note : notes)
			{
				if (counter == 0)
				{
					this->NoteOff(static_cast<std::uint32_t>(ifOverlap.second - curTick), note.note);
				}
				else { this->NoteOff(0, note.note); }
				++counter;
			}
		};

	(([this, &args, &forloop]()
		{
			if (!args.ifOverlap.first)
			{
				for (const Note& note : args.notes)
				{
					this->NoteOn(note.deltaStart, note.note, note.velocity);
					this->NoteOff(note.deltaEnd, note.note);
				}
			}
			else { forloop(args.notes, args.ifOverlap); }
		})(), ...);
}

// Writes delta, event, and the variadic arguments into the file.
template<typename ...Ts>
inline void MidiFile::WriteEvent(std::uint32_t delta, std::uint8_t Event, const Ts& ...args)
{
	WriteVarLength(delta);
	Write(Event);
	((Write(args)), ...);
}

// Takes anything, casts it into const char*, and puts the value into the file using std::ofstream::write
template<typename... Ts>
inline void MidiFile::Write(Ts&&... args)
{
	((m_file.write(reinterpret_cast<const char*>(&args), sizeof(args))), ...);
}

// Casts the arguments into a char and puts the value into the file using std::ofstream::put 
template<typename ...Ts>
inline void MidiFile::Put(Ts&& ...args)
{
	((m_file.put(static_cast<char>(args))), ...);
}

// Assigns to original variadic arguments byteswapped values using std::byteswap. 
template <typename... Ts>
inline void MidiFile::ByteSwap(Ts&... args)
{
	((args = std::byteswap(args)), ...);
}
