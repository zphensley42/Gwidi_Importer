#ifndef GWIDI_IMPORTER_GWIDIDATA_H
#define GWIDI_IMPORTER_GWIDIDATA_H

#include <vector>
#include <string>
#include <unordered_map>


namespace gwidi {
namespace data {

struct SlotInfo {
    int octave;
    int measure_index;
    std::string note_key;
    int length_in_slots;
    int start_in_slots;  // slot index, 0 based
    int channel;
    std::string instrument;
    int track;
    bool is_held;
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
private:
    friend class Song;
    std::string m_instrument;
    int m_channel;
    int m_trackNum;

    std::vector<Measure> m_measures;
};

class Song {
public:
    void addSlot(SlotInfo slot);

    int measureCount();
    std::vector<Slot> slotsForParams(int track, int octave, const std::string& note);
    std::vector<VerticalSlotRepr> slotsForParams(int track, int slotIndex);
private:
    std::vector<Track> m_tracks;
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
