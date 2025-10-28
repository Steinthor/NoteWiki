#include <gtest/gtest.h>

#include "note.hpp"

// Test note creation, tagging, content manipulation
// Test visibility and editing flags
TEST(NoteTest, DefaultState) {
    Note note;
    std::string t1 = "test1";
    std::string t2 = "test2";

    EXPECT_EQ(note.title, "");
    EXPECT_EQ(note.content, "");
    EXPECT_TRUE(note.tags.empty());
    EXPECT_TRUE(note.children.empty());
    EXPECT_FALSE(note.show_note);
    EXPECT_FALSE(note.edit_text);
}

TEST(NoteTest, StringAssignment) {
    Note note;
    std::string t1 = "test1";
    std::string t2 = "test2";

    note.title = t1;
    EXPECT_EQ(note.title, t1);
    note.content = t2;
    EXPECT_EQ(note.content, t2);
}

TEST(NoteTest, TagOperations) {
    Note note;
    std::string t1 = "tag1";
    std::string t2 = "tag2";

    note.tags.emplace_back(t1);
    note.tags.emplace_back(t2);
    EXPECT_EQ(note.tags.size(), 2);
    EXPECT_EQ(note.tags[0], t1);
    EXPECT_EQ(note.tags[1], t2);
    
    // Test clear operation
    note.tags.clear();
    EXPECT_TRUE(note.tags.empty());
}

TEST(NoteTest, ChildOperations) {
    Note note;
    std::string t1 = "child1";
    std::string t2 = "child2";

    note.children.emplace_back(t1);
    note.children.emplace_back(t2);
    EXPECT_EQ(note.children.size(), 2);
    EXPECT_EQ(note.children[0], t1);
    EXPECT_EQ(note.children[1], t2);
}

TEST(NoteTest, FlagOperations) {
    Note note;

    // Test initial flag states
    EXPECT_FALSE(note.show_note);
    EXPECT_FALSE(note.edit_text);
    
    // Test setting flags
    note.show_note = true;
    note.edit_text = true;
    EXPECT_TRUE(note.show_note);
    EXPECT_TRUE(note.edit_text);
}

TEST(NoteTest, CopySemantics) {
    Note original;
    std::string title = "Original Title";
    std::string tag = "tag";
    std::string child = "child";
    original.title = title;
    original.tags.emplace_back(tag);
    original.children.emplace_back(child);
    original.show_note = true;
    
    // Test copy constructor
    Note copy = original;
    EXPECT_EQ(copy.title, title);
    EXPECT_EQ(copy.tags.size(), 1);
    EXPECT_EQ(copy.children.size(), 1);
    EXPECT_TRUE(copy.show_note);
    
    // Test assignment operator
    Note assigned;
    assigned = original;
    EXPECT_EQ(assigned.title, title);
    EXPECT_EQ(assigned.tags.size(), 1);
    EXPECT_EQ(assigned.children.size(), 1);
    EXPECT_TRUE(assigned.show_note);
}

TEST(NoteTest, MemoryUsage) {
    Note note;
    note.title = std::string(1000, 'a');  // Large string
    note.content = std::string(1000, 'b');
    EXPECT_EQ(note.title.size(), 1000);
    EXPECT_EQ(note.content.size(), 1000);
}

// Test NoteStore functionality
// Test loading from valid JSON
// Test handling of missing/invalid files
// Test note creation and manipulation
TEST(NoteStoreTest, LoadNotesFromFile) {
    NoteStore manager;
}