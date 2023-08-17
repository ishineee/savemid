#include "MidiFile.h"
#include <bit>
#include <array>


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
	std::array<int, 4> buffer;
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

void MidiFile::StartFile()
{
	std::uint8_t division{ static_cast<std::uint8_t>(m_delta) };
	ByteSwap(division);
	m_file.write("MThd", 4);
	Put(0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00);
	Write(division);
}

void MidiFile::CalculateTrackSize()
{
	auto val = static_cast<std::uint32_t>(m_trackPos);
	auto val2 = static_cast<std::uint32_t>(m_file.tellp());
	std::uint32_t size = val2 - val - 8;
	m_file.seekp(m_trackPos);
	ByteSwap(size);
	Write(size);
}

void MidiFile::SetTempo(const std::uint32_t bpm)
{
	Put(0x00, MetaEvents::StartMeta, MetaEvents::SetTempoEvent, MetaEvents::SetBPM);
	std::uint32_t bpmcalc = 60000000 / bpm;
	Put((bpmcalc >> 16) & 0xFF, (bpmcalc >> 8) & 0xFF, bpmcalc & 0xFF);
}

void MidiFile::EndFile()
{
	Put(0x00, MetaEvents::StartMeta, MetaEvents::EndTrack, 0x00);
	CalculateTrackSize();
}

void MidiFile::NoteOn(const std::uint32_t delta, const Notes note, const std::uint8_t velocity)
{
	WriteEvent(
		delta,
	    static_cast<std::uint8_t>(Events::NoteOn),
		static_cast<std::uint8_t>(note),
		velocity
	);
}

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
