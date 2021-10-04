#include <gtest/gtest.h>
#include "GwidiData.h"


TEST(GwidiTest, SimpleImport) {
    auto isIndexActivated = [](std::vector<gwidi::data::Slot>& vector, int index) {
        if(index >= vector.size()) {
            return false;
        }
        return vector[index].state() == gwidi::data::Slot::State::SLOT_ACTIVATED;
    };

    auto isIndexHeld = [](std::vector<gwidi::data::Slot>& vector, int index) {
        if(index >= vector.size()) {
            return false;
        }
        return vector[index].state() == gwidi::data::Slot::State::SLOT_HELD;
    };

    auto getActiveNoteLength = [](std::vector<gwidi::data::Slot>& vector, int index) {
        if(index >= vector.size()) {
            return 0;
        }

        auto& start = vector[index];
        if(start.state() != gwidi::data::Slot::State::SLOT_ACTIVATED) {
            return 0;
        }

        int length = 0;
        while(start.state() != gwidi::data::Slot::State::SLOT_NONE) {
            index++;
            start = vector[index];
            length++;
        }

        return length;
    };

    auto importedData = gwidi::data::Importer::instance().import("./Build/test_data/Debug/simple.mid");

    // Horizontal slices
    auto octave4_noteb_slots = importedData->slotsForParams(0, 4, "B");
    auto octave4_notea_slots = importedData->slotsForParams(0, 4, "A");
    auto octave4_noteg_slots = importedData->slotsForParams(0, 4, "G");
    auto octave4_notef_slots = importedData->slotsForParams(0, 4, "F");
    auto octave4_notedsharp_slots = importedData->slotsForParams(0, 4, "D"); // D# == D
    auto octave4_notecsharp_slots = importedData->slotsForParams(0, 4, "C1"); // C# == C
    ASSERT_EQ(3, importedData->measureCount());
    // Make assertions against this imported data

    // 1 note at octave 4, note b, length 1, start 0
    ASSERT_TRUE(isIndexActivated(octave4_noteb_slots, 0));
    ASSERT_EQ(1, getActiveNoteLength(octave4_noteb_slots, 0));

    // 1 note at octave 4, note b, length 1, start 2
    ASSERT_TRUE(isIndexActivated(octave4_noteb_slots, 2));
    ASSERT_EQ(1, getActiveNoteLength(octave4_noteb_slots, 2));

    // 1 note at octave 4, note a, length 28, start 4
    ASSERT_TRUE(isIndexActivated(octave4_notea_slots, 4));
    ASSERT_EQ(28, getActiveNoteLength(octave4_notea_slots, 4));

    // 1 note at octave 4, note g, length 4, start 34
    ASSERT_TRUE(isIndexActivated(octave4_noteg_slots, 34));
    ASSERT_EQ(4, getActiveNoteLength(octave4_noteg_slots, 34));

    // 1 note at octave 4, note f, length 4, start 35
    ASSERT_TRUE(isIndexActivated(octave4_notef_slots, 35));
    ASSERT_EQ(4, getActiveNoteLength(octave4_notef_slots, 35));

    // 1 note at octave 4, note d#, length 4, start 35
    ASSERT_TRUE(isIndexActivated(octave4_notedsharp_slots, 35));
    ASSERT_EQ(4, getActiveNoteLength(octave4_notedsharp_slots, 35));

    // 1 note at octave 4, note c#, length 4, start 36
    ASSERT_TRUE(isIndexActivated(octave4_notecsharp_slots, 36));
    ASSERT_EQ(4, getActiveNoteLength(octave4_notecsharp_slots, 36));


    // Vertical slices
    auto findNoteInSlice = [](std::vector<gwidi::data::VerticalSlotRepr>& slice, int octave, const std::string &note) -> gwidi::data::Slot* {
        auto it = std::find_if(slice.begin(), slice.end(), [&octave, &note](gwidi::data::VerticalSlotRepr &repr) {
            return repr.octave_num == octave && repr.note == note;
        });
        if(it != slice.end()) {
            return &(it->slot);
        }
        return nullptr;
    };

    auto slice1 = importedData->slotsForParams(0, 44);
    ASSERT_NE(nullptr, findNoteInSlice(slice1, 7, "B"));
    ASSERT_EQ(gwidi::data::Slot::State::SLOT_NONE, findNoteInSlice(slice1, 9, "B")->state());
    ASSERT_EQ(gwidi::data::Slot::State::SLOT_NONE, findNoteInSlice(slice1, 8, "B")->state());
    ASSERT_EQ(gwidi::data::Slot::State::SLOT_NONE, findNoteInSlice(slice1, 7, "D")->state());
    ASSERT_EQ(gwidi::data::Slot::State::SLOT_ACTIVATED, findNoteInSlice(slice1, 7, "B")->state());
    ASSERT_EQ(gwidi::data::Slot::State::SLOT_ACTIVATED, findNoteInSlice(slice1, 6, "B")->state());
    ASSERT_EQ(gwidi::data::Slot::State::SLOT_ACTIVATED, findNoteInSlice(slice1, 5, "B")->state());
    ASSERT_EQ(gwidi::data::Slot::State::SLOT_ACTIVATED, findNoteInSlice(slice1, 4, "B")->state());
    ASSERT_EQ(gwidi::data::Slot::State::SLOT_ACTIVATED, findNoteInSlice(slice1, 3, "B")->state());
    ASSERT_EQ(gwidi::data::Slot::State::SLOT_ACTIVATED, findNoteInSlice(slice1, 2, "B")->state());
    ASSERT_EQ(gwidi::data::Slot::State::SLOT_NONE, findNoteInSlice(slice1, 1, "B")->state());
}
