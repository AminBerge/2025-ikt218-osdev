extern "C"{
    #include "libc/system.h"
    #include "memory/memory.h"
    #include "common.h"
    #include "interrupts.h"
    #include "input.h"
    #include "song/song.h"
}

// Existing global operator new overloads
void* operator new(size_t size) {
    return malloc(size);
}

void* operator new[](size_t size) {
    return malloc(size);
}

// Existing global operator delete overloads
void operator delete(void* ptr) noexcept {
    free(ptr);
}

void operator delete[](void* ptr) noexcept {
    free(ptr);
}

// Add sized-deallocation functions
void operator delete(void* ptr, size_t size) noexcept {
    (void)size; // Size parameter is unused, added to match required signature
    free(ptr);
}

void operator delete[](void* ptr, size_t size) noexcept {
    (void)size; // Size parameter is unused, added to match required signature
    free(ptr);
}

// Global state for music player control
SongPlayer* g_player = nullptr;
Song** g_songs = nullptr;
uint32_t g_n_songs = 0;
uint32_t g_current_song = 0;
bool g_is_playing = false;
volatile bool g_should_stop = false;  // Flag to signal song interruption

// Function prototypes for music control
void play_current_song();
void pause_song(); 
void resume_song();
void next_song();
void reset_player();
void prev_song();
void debug_song(Song* song);

SongPlayer* create_song_player() {
    auto* player = new SongPlayer();
    player->play_song = play_song_impl;
    return player;
}

extern "C" int kernel_main(void);
int kernel_main(){
    // Set up interrupt handlers
    register_interrupt_handler(3, [](registers_t* regs, void* context) {
        printf("Interrupt 3 - OK\n");
    }, NULL);
    
    register_interrupt_handler(4, [](registers_t* regs, void* context) {
        printf("Interrupt 4 - OK\n");
    }, NULL);
    
    register_interrupt_handler(14, [](registers_t* regs, void* context) {
        uint32_t faulting_address;
        __asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));
        
        int32_t present = !(regs->err_code & 0x1);
        int32_t rw = regs->err_code & 0x2;
        int32_t us = regs->err_code & 0x4;
        int32_t reserved = regs->err_code & 0x8;
        int32_t id = regs->err_code & 0x10;
        
        printf("Page fault! (");
        if (present)
            printf("present ");
        if (rw)
            printf("read-only ");
        if (us)
            printf("user-mode ");
        if (reserved)
            printf("reserved ");
        printf(") at address 0x%x\n", faulting_address);
        panic("Page fault");
    }, NULL);
    
    // Initialize songs - do this first to ensure memory is properly allocated
    g_songs = new Song*[8] {
        new Song{battlefield_1942_theme, sizeof(battlefield_1942_theme) / sizeof(Note)},
        new Song{starwars_theme, sizeof(starwars_theme) / sizeof(Note)},
        new Song{music_1, sizeof(music_1) / sizeof(Note)},
        new Song{music_6, sizeof(music_6) / sizeof(Note)},
        new Song{music_5, sizeof(music_5) / sizeof(Note)},
        new Song{music_4, sizeof(music_4) / sizeof(Note)},
        new Song{music_3, sizeof(music_3) / sizeof(Note)},
        new Song{music_2, sizeof(music_2) / sizeof(Note)}
    };
    g_n_songs = 8;
    g_current_song = 0;
    
    // Create a song player
    g_player = create_song_player();
    
    // Trigger interrupts to test handlers
    asm volatile ("int $0x3");
    asm volatile ("int $0x4");
    
    // Register IRQ handler for keyboard (IRQ1) - after songs are initialized
    register_irq_handler(IRQ1, [](registers_t*, void*) {
        // Read from keyboard
        unsigned char scan_code = inb(0x60);
        char key = scancode_to_ascii(&scan_code);
        
        // Handle music control keys
        switch(key) {
            case 'p': // Pause/Resume
                if (g_is_playing) {
                    pause_song();
                    printf("[PAUSED] Press 'p' to resume, 'n' for next song, 'b' for previous\n");
                } else {
                    resume_song();
                }
                break;
            case 'n': // Next song
                next_song();
                printf("[NEXT] Playing song %d of %d\n", g_current_song + 1, g_n_songs);
                break;
            case 'b': // Previous song
                prev_song();
                printf("[PREV] Playing song %d of %d\n", g_current_song + 1, g_n_songs);
                break;
            case 'r': // Reset player state (emergency reset)
                reset_player();
                printf("[RESET] Player reset. Press 'p' to start playing\n");
                break;
            case 'd': // Debug current song
                if (g_current_song < g_n_songs && g_songs[g_current_song] != nullptr) {
                    printf("[DEBUG] Analyzing song %d of %d\n", g_current_song + 1, g_n_songs);
                    debug_song(g_songs[g_current_song]);
                } else {
                    printf("[DEBUG] No valid song to analyze\n");
                }
                break;
            default:
                printf("%c", key);
                break;
        }
    }, NULL);
    
    // Enable interrupts
    asm volatile("sti");
    
    printf("Music Player Ready\n");
    printf("Controls:\n");
    printf(" 'p' - Play/Pause\n");
    printf(" 'n' - Next song\n");
    printf(" 'b' - Previous song\n");
    printf(" 'r' - Reset player\n");
    printf(" 'd' - Debug current song\n");
    printf("Press 'p' to start playing song 1 of %d\n", g_n_songs);
    
    // Main loop
    printf("Kernel main loop\n");
    while(true) {
        // Allow system to process interrupts
        asm volatile("hlt");
    }
    
    // This part will not be reached
    printf("Done!\n");
    return 0;
}

// Music control functions
void debug_song(Song* song) {
    if (!song) {
        printf("DEBUG: Null song pointer\n");
        return;
    }
    
    printf("DEBUG: Song length=%d\n", song->length);
    
    // Print the first few notes
    uint32_t debug_count = song->length > 5 ? 5 : song->length;
    for (uint32_t i = 0; i < debug_count; i++) {
        printf("Note %d: Freq=%d, Dur=%d\n", 
               i, song->notes[i].frequency, song->notes[i].duration);
    }
}

void reset_player() {
    // Reset the player state
    g_should_stop = false;
    g_is_playing = false;
}

void play_current_song() {
    if (g_current_song < g_n_songs && g_songs[g_current_song] != nullptr) {
        
        g_should_stop = false;  // Make sure stop flag is cleared
        g_is_playing = true;
        
        printf("[PLAYING] Song %d of %d\n", g_current_song + 1, g_n_songs);
        
        // Check song validity
        Song* current_song = g_songs[g_current_song];
        
        // Debug song data
        debug_song(current_song);
        
        if (current_song->length > 0) {
            // Play the song
            g_player->play_song(current_song);
        } else {
            printf("Error: Song has no notes!\n");
        }
        
        // Only proceed with auto-advancing if we weren't interrupted
        if (!g_should_stop) {
            g_is_playing = false;
            
            // Auto-advance to next song when current song finishes
            if (g_current_song < g_n_songs - 1) {
                g_current_song++;
                printf("Song finished. Press 'p' to play next song (%d of %d)\n", 
                      g_current_song + 1, g_n_songs);
            } else {
                g_current_song = 0;
                printf("Playlist finished. Press 'p' to restart from first song\n");
            }
        }
    } else {
        printf("Error: Invalid song index or null song pointer!\n");
        g_is_playing = false;
    }
}

void pause_song() {
    if (g_is_playing) {
        g_should_stop = true;  // Signal the player to stop
        g_is_playing = false;
        
        // Wait for a small moment to ensure the signal is processed
        for (int i = 0; i < 1000; i++) { 
            /* Short delay - avoid volatile increment warning */
            asm volatile("nop");
        }
    }
}

void resume_song() {
    if (!g_is_playing) {
        g_is_playing = true;
        play_current_song();
    }
}

void next_song() {
    // Stop current song if playing
    if (g_is_playing) {
        pause_song();
    }
    
    // Wait a moment for the song to stop properly
    for (int i = 0; i < 10000; i++) { 
        /* Short delay - avoid volatile increment warning */
        asm volatile("nop");
    }
    
    // Advance to next song
    if (g_current_song < g_n_songs - 1) {
        g_current_song++;
    } else {
        g_current_song = 0; // Wrap around to the first song
    }
    
    // Reset player state
    reset_player();
    
    // Start playing the new song
    resume_song();
}

void prev_song() {
    // Stop current song if playing
    if (g_is_playing) {
        pause_song();
    }
    
    // Wait a moment for the song to stop properly
    for (int i = 0; i < 10000; i++) { 
        /* Short delay - avoid volatile increment warning */
        asm volatile("nop");
    }
    
    // Go to previous song
    if (g_current_song > 0) {
        g_current_song--;
    } else {
        g_current_song = g_n_songs - 1; // Wrap around to the last song
    }
    
    // Reset player state
    reset_player();
    
    // Start playing the new song
    resume_song();
}