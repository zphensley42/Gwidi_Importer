#include "GwidiData.h"
#include "MidiFile.h"
#include <string>
#include <iostream>
#include <algorithm>

namespace gwidi {
namespace data {

Slot::Slot(const gwidi::data::Slot &other) {
    m_state = other.m_state;
}

Note::Note(int index) {
    // Fill slits (16 slots per measure per note)
    for(int i = 0; i < 16; i++) {
        m_slots.emplace_back(Slot());
    }

    static std::vector<std::string> note_keys {
        "C2",
        "B",
        "A",
        "G",
        "F",
        "E",
        "D",
        "C1",
    };

    m_noteKey = note_keys[index];
}

Octave::Octave(int octaveNum) : m_octaveNum{octaveNum} {
    // Fill notes (8 notes per octave)
    for(int i = 0; i < 8; i++) {
        m_notes.emplace_back(Note(i));
    }
}

Measure::Measure() {
    // Fill octaves with possibilities
    // -1 -> 9
    for(int i = -1; i <= 9; i++) {
        m_octaves.emplace_back(Octave(i));
    }
}

void Note::addSlot(SlotInfo &slot) {
    int slot_index = slot.start_in_slots % 16;
    if(slot_index < m_slots.size()) {
        m_slots[slot_index].m_state = slot.is_held ? Slot::State::SLOT_HELD : Slot::State::SLOT_ACTIVATED;
    }
}

void Octave::addSlot(SlotInfo &slot) {
    // Determine which note the slot is for
    auto it = std::find_if(m_notes.begin(), m_notes.end(), [&slot](Note& note){
        return slot.note_key == note.m_noteKey;
    });
    if(it != m_notes.end()) {
        it->addSlot(slot);
    }
}

void Measure::addSlot(SlotInfo &slot) {
    auto it = std::find_if(m_octaves.begin(), m_octaves.end(), [&slot](Octave &o){
        return o.m_octaveNum == slot.octave;
    });
    if(it != m_octaves.end()) {
        it->addSlot(slot);
    }
}

std::vector<Slot> Measure::slotsForParams(int octave, const std::string &note) {
    auto octaveIt = std::find_if(m_octaves.begin(), m_octaves.end(), [&octave](const Octave& o) {
       return o.m_octaveNum == octave;
    });
    if(octaveIt == m_octaves.end()) {
        return {};
    }

    auto &o = *octaveIt;
    auto it = std::find_if(o.m_notes.begin(), o.m_notes.end(), [&note](Note& n){
        return n.key() == note;
    });

    if(it != o.m_notes.end()) {
        return it->slots();
    }
    return {};
}

std::vector<VerticalSlotRepr> Measure::slotsForParams(int slotIndex) {
    std::vector<VerticalSlotRepr> r;
    for(auto& octave : m_octaves) {
        for(auto& note : octave.m_notes) {
            VerticalSlotRepr repr {
                octave.octaveNum(),
                note.key(),
                note.slots()[slotIndex]
            };
            r.emplace_back(repr);
        }
    }
    return r;
}

// TODO: The # of measures being added is incorrect for some reason, revisit this logic
void Track::addSlot(SlotInfo &slot) {
    // When adding a slot to the track, cycle through the 'slots' defined by the length information in the slot
    // Slots other than start should be is_held==true
    int numSlotsToAdd = slot.length_in_slots;
    for(int i = 0; i < numSlotsToAdd; i++) {

        int to_start = slot.start_in_slots + i;
        int measure_index = to_start / 16;  // num of slots per measure

        SlotInfo slotIteration = slot; // copy
        slotIteration.start_in_slots = to_start;
        slotIteration.measure_index = measure_index;

        // Only the first note is activated, the rest are held
        if(i > 0) {
            slotIteration.is_held = true;
        }

        if(measure_index >= m_measures.size()) {
            int diff = (measure_index + 1) - m_measures.size();
            for(int i = 0; i < diff; i++) {
                m_measures.emplace_back(Measure());
            }
        }

        m_measures[measure_index].addSlot(slotIteration);
    }
}

void Song::emptyInit(int trackCount, int measureCount) {
    for(int i = 0; i < trackCount; i++) {
        m_tracks.emplace_back(Track{});
        for(int m = 0; m < measureCount; m++) {
            m_tracks.back().m_measures.emplace_back(Measure{});
        }
    }
}

void Song::addSlot(SlotInfo slot) {
    Track* track{nullptr};
    auto it = std::find_if(m_tracks.begin(), m_tracks.end(), [&slot](Track& track) {
        return track.m_trackNum == slot.track;
    });

    if(it == m_tracks.end()) {
        m_tracks.emplace_back(Track());
        track = &m_tracks.back();
    }
    else {
        track = &(*it);
    }

    if(track) {
        track->m_instrument = slot.instrument;
        track->m_channel = slot.channel;
        track->m_trackNum = slot.track;
        track->addSlot(slot);
    }
}

int Song::measureCount() {
    if(m_tracks.empty()) {
        return 0;
    }
    return m_tracks.front().m_measures.size();
}

std::vector<Slot> Song::slotsForParams(int track, int octave, const std::string& note) {
    std::vector<Slot> slots;
    if(track >= m_tracks.size()) {
        return slots;
    }
    for(auto& m : m_tracks[track].m_measures) {
        auto s = m.slotsForParams(octave, note);
        for(auto& slot : s) {
            slots.emplace_back(slot);
        }
    }

    return slots;
}

std::vector<VerticalSlotRepr> Song::slotsForParams(int track, int slotIndex) {
    if(track >= m_tracks.size()) {
        return {};
    }

    int measureIndex = slotIndex / 16;
    int subSlotIndex = slotIndex % 16;

    if(measureIndex >= m_tracks[track].m_measures.size()) {
        return {};
    }
    return m_tracks[track].m_measures[measureIndex].slotsForParams(subSlotIndex);
}

Importer & Importer::instance() {
    static Importer s;
    return s;
}

Song* Importer::import(const std::string &fname) {
    auto song = new Song();


    smf::MidiFile midifile(fname.c_str());

//    midifile.doTimeAnalysis();
    auto linkedNoteCount = midifile.linkNotePairs();
    std::cout << "linkedNoteCount: " << linkedNoteCount << std::endl;

    std::string instrument = "";
    int instrumentChannel = -1;
    double tempo = 120;
    auto ticksPerBeat = midifile.getTicksPerQuarterNote();
    int ticksPerSixteenth = ticksPerBeat / 4;
    int tracks = midifile.getTrackCount();

    int ticksPerMeasure = ticksPerBeat * 4;  // 4/4 time (4 quarter notes per measure)
    std:: cout << "ticksPerBeat: " << ticksPerBeat << ", ticksPerMeasure: " << ticksPerMeasure << ", ticksPerSixteenth: " << ticksPerSixteenth << std::endl;

    for(int t = 0; t < tracks; t++) {
        auto &track = midifile[t];
        int events = track.getEventCount();

        for(size_t e = 0; e < events; e++) {
            auto &event = track[e];

            if(event.isProgramChange()) {
                instrument = event.getInstrument();
                instrumentChannel = event.getProgramChannel();
            }

            if(event.isTempo()) {
                tempo = event.getTempoBPM();
                std::cout << "detected tempo: " << tempo << std::endl;
            }

            if(event.isNoteOn()) {
                auto note_key = event.getKeyNumber();
                auto note_ticks = event.getTickDuration();
                auto note_tick_start = event.tick;
                auto note_start = event.tick / ticksPerSixteenth;  // in # of sixteenth notes

                // Length is number of sixteenth notes
                int note_length = note_ticks / ticksPerSixteenth;
                std::string note_letter = translateKeyVal(note_key);
                int note_octave = event.getKeyOctave();

                std::cout << "Note{track: " << t
                    << ", instrument: " << instrument
                    << ", key: " << note_key
                    << ", note_ticks: " << note_ticks
                    << ", tick_position: " << note_tick_start
                    << ", note_start: " << note_start
                    << ", note_length: " << note_length
                    << ", note_letter: " << note_letter
                    << ", note_octave: " << note_octave
                << "}\n";
                std::cout << "{Instrument: " << instrument << ", instrumentChannel: " << instrumentChannel << "}\n";

                // Determine our indices (add measures if needed)
                // Use time for measure / slot, note_key for octave
                int ticks_per_measure = 16;
                int measure_index = note_start / ticks_per_measure;

                song->setTempo(tempo);
                song->addSlot({
                    note_octave,        // int octave;
                    measure_index,      // int measure_index;
                    note_letter,        // std::string key;
                    note_length,        // int length_in_slots;
                    note_start,         // int start_in_slots;  // slot index, 0 based
                    instrumentChannel,  // int channel;
                    instrument,         // std::string instrument;
                    t,                  // int track;
                    false       // bool is_held;
                });
            }
        }
    }

    // TODO: Unit test cases -- test held notes across measures
    return song;
}


// Octave #	Note Numbers
//      C	C#	D	D#	E	F	F#	G	G#	A	A#	B
// -1	0	1	2	3	4	5	6	7	8	9	10	11
// 0	12	13	14	15	16	17	18	19	20	21	22	23
// 1	24	25	26	27	28	29	30	31	32	33	34	35
// 2	36	37	38	39	40	41	42	43	44	45	46	47
// 3	48	49	50	51	52	53	54	55	56	57	58	59
// 4	60	61	62	63	64	65	66	67	68	69	70	71
// 5	72	73	74	75	76	77	78	79	80	81	82	83
// 6	84	85	86	87	88	89	90	91	92	93	94	95
// 7	96	97	98	99	100	101	102	103	104	105	106	107
// 8	108	109	110	111	112	113	114	115	116	117	118	119
// 9	120	121	122	123	124	125	126	127


std::string Importer::translateKeyVal(int keyVal) {
    static std::unordered_map<std::string, std::vector<int>> keyLetters {
            {"C1", std::vector<int>{0, 1, 12, 13, 24, 25, 36, 37, 48, 49, 60, 61, 72, 73, 84, 85, 96, 97, 108, 109, 120, 121}},
            {"D", std::vector<int>{2, 3, 14, 15, 26, 27, 38, 39, 50, 51, 62, 63, 74, 75, 86, 87, 98, 99, 110, 111, 122, 123}},
            {"E", std::vector<int>{4, 16, 28, 40, 52, 64, 76, 88, 100, 112, 124}},
            {"F", std::vector<int>{5, 6, 17, 18, 29, 30, 41, 42, 53, 54, 65, 66, 77, 78, 89, 90, 101, 102, 113, 114, 125, 126}},
            {"G", std::vector<int>{7, 8, 19, 20, 31, 32, 43, 44, 55, 56, 67, 68, 79, 80, 91, 92, 103, 104, 115, 116}},
            {"A", std::vector<int>{9, 10, 21, 22, 33, 34, 45, 46, 57, 58, 69, 70, 81, 82, 93, 94, 105, 106, 117, 118}},
            {"B", std::vector<int>{11, 23, 35, 47, 59, 71, 83, 95, 107, 119}},
            {"C2", {}},
    };

    for(auto &entry : keyLetters) {
        auto it = std::find(entry.second.begin(), entry.second.end(), keyVal);
        if(it != entry.second.end()) {
            return entry.first;
        }
    }

    return "";
}

}
}
