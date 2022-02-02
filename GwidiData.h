#ifndef GWIDI_IMPORTER_GWIDIDATA_H
#define GWIDI_IMPORTER_GWIDIDATA_H

#define DEBUG_SAVE_LOAD true

#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>


namespace gwidi {
namespace data {

struct SlotInfo {
    int octave{0};
    int measure_index{0};
    std::string note_key{0};
    int length_in_slots{0};
    int start_in_slots{0};  // slot index, 0 based
    int channel{0};
    std::string instrument{""};
    int track{0};
    bool is_held{false};
    bool is_activated{true};
    SlotInfo() = default;
    SlotInfo(
            int octave,
            int measure_index,
            std::string note_key,
            int length_in_slots,
            int start_in_slots,
            int channel,
            std::string instrument,
            int track,
            bool is_held,
            bool is_activated) :
            octave{octave},
            measure_index{measure_index},
            note_key{note_key},
            length_in_slots{length_in_slots},
            start_in_slots{start_in_slots},
            channel{channel},
            instrument{instrument},
            track{track},
            is_held{is_held},
            is_activated{is_activated} {
    }
};

class Slot {
public:
    enum State {
        SLOT_NONE = 0,
        SLOT_ACTIVATED,
        SLOT_HELD,
    };
    Slot() = default;
    Slot(const Slot& other);

    inline State state() {
        return m_state;
    }
private:
    friend class Note;
    State m_state;
};

struct VerticalSlotRepr {
    int octave_num;
    std::string note;
    Slot slot;
};


class Note {
public:
    Note(int index);
    void addSlot(SlotInfo &slot);

    inline std::string key() {
        return m_noteKey;
    }

    inline std::vector<Slot> slots() {
        return m_slots;
    }
private:
    friend class Octave;
    std::string m_noteKey;
    std::vector<Slot> m_slots;
};

class Octave {
public:
    Octave(int octaveNum);
    void addSlot(SlotInfo &slot);

    inline int octaveNum() {
        return m_octaveNum;
    }

    inline std::vector<Note>& notes() {
        return m_notes;
    }
private:
    friend class Measure;
    int m_octaveNum;
    std::vector<Note> m_notes;
};

class Measure {
public:
    Measure();
    void addSlot(SlotInfo &slot);
    std::vector<Slot> slotsForParams(int octave, const std::string& note);
    std::vector<VerticalSlotRepr> slotsForParams(int slotIndex);

    inline std::vector<Octave>& octaves() {
        return m_octaves;
    }
private:
    std::vector<Octave> m_octaves;
};

class Track {
public:
    Track() = default;
    void addSlot(SlotInfo &slot);

    inline std::string instrument() {
        return m_instrument;
    }

    inline int channel() {
        return m_channel;
    }

    inline std::vector<Measure>& measures() {
        return m_measures;
    }
private:
    friend class Song;
    std::string m_instrument;
    int m_channel;
    int m_trackNum;

    std::vector<Measure> m_measures;
};

class Song {
public:
    Song() = default;
    void emptyInit(int trackCount, int measureCount);
    void addSlot(SlotInfo slot);
    inline void setTempo(int tempo) {
        m_tempo = tempo;
    }
    inline int tempo() const {
        return m_tempo;
    }

    int measureCount();
    std::vector<Slot> slotsForParams(int track, int octave, const std::string& note);
    std::vector<VerticalSlotRepr> slotsForParams(int track, int slotIndex);
    inline std::vector<Track>& tracks() {
        return m_tracks;
    }

    void saveAsBinary(std::ostream& os, int selectedTrack);
private:
#ifdef DEBUG_SAVE_LOAD
    static std::ofstream& debugOutFile(bool open = false);
#endif

    void writeToStream(const std::string& label, int64_t labelData, std::ostream& os, const char* data, std::streamsize n) {
        std::stringstream ss;
        ss << labelData;
        writeToStream(label, ss.str(), os, data, n);
    }
    void writeToStream(const std::string& label, const std::string& labelData, std::ostream& os, const char* data, std::streamsize n);

    std::vector<Track> m_tracks;
    int m_tempo;
};

class Importer {
public:
    static Importer& instance();
    Song* import(const std::string &fname);
private:
    std::string translateKeyVal(int keyVal);
};

}
}


#endif //GWIDI_IMPORTER_GWIDIDATA_H
