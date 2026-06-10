#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define WIDTH 80
#define HEIGHT 20
#define MAX_SHAPES 100
// ANSI Color Codes for premium CLI experience
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
typedef enum {
    SHAPE_LINE = 1,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;
typedef struct {
    int x1, y1, x2, y2;
} Line;
typedef struct {
    int x, y, w, h;
} Rectangle;
typedef struct {
    int cx, cy, r;
} Circle;
typedef struct {
    int x1, y1, x2, y2, x3, y3;
} Triangle;
typedef struct {
    int id;
    ShapeType type;
    union {
        Line line;
        Rectangle rect;
        Circle circle;
        Triangle tri;
    } data;
    char ch;
} Shape;
// Global structures
char canvas[HEIGHT][WIDTH];
Shape shapes[MAX_SHAPES];
int shape_count = 0;
int next_id = 1;
// --- Helper Functions for Safe Terminal Input ---
int read_int_range(const char* prompt, int min_val, int max_val) {
    int val;
    char buffer[100];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }
        buffer[strcspn(buffer, "\n")] = '\0';
        char* endptr;
        val = (int)strtol(buffer, &endptr, 10);
        if (endptr == buffer || *endptr != '\0') {
            printf(RED "Invalid input. Please enter a valid integer.\n" RESET);
            continue;
        }
        if (val < min_val || val > max_val) {
            printf(RED "Value out of range (%d to %d). Please try again.\n" RESET, min_val, max_val);
            continue;
        }
        return val;
    }
}
char read_char(const char* prompt, char default_char) {
    char buffer[100];
    printf("%s (default '%c'): ", prompt, default_char);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return default_char;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    if (strlen(buffer) == 0) {
        return default_char;
    }
    return buffer[0];
}
// --- Canvas Drawing Operations ---
void draw_point(int x, int y, char ch) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        canvas[y][x] = ch;
    }
}
// Bresenham's Line Algorithm
void draw_line(int x1, int y1, int x2, int y2, char ch) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;
    
    while (1) {
        draw_point(x1, y1, ch);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}
// Rectangle Drawing (outline only)
void draw_rectangle(int x, int y, int w, int h, char ch) {
    for (int col = x; col < x + w; col++) {
        draw_point(col, y, ch);
        draw_point(col, y + h - 1, ch);
    }
    for (int row = y; row < y + h; row++) {
        draw_point(x, row, ch);
        draw_point(x + w - 1, row, ch);
    }
}
// Midpoint Circle Algorithm
void draw_circle(int cx, int cy, int r, char ch) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    
    while (y >= x) {
        draw_point(cx + x, cy + y, ch);
        draw_point(cx - x, cy + y, ch);
        draw_point(cx + x, cy - y, ch);
        draw_point(cx - x, cy - y, ch);
        draw_point(cx + y, cy + x, ch);
        draw_point(cx - y, cy + x, ch);
        draw_point(cx + y, cy - x, ch);
        draw_point(cx - y, cy - x, ch);
        
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}
// Triangle Drawing (connecting vertices with 3 lines)
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, char ch) {
    draw_line(x1, y1, x2, y2, ch);
    draw_line(x2, y2, x3, y3, ch);
    draw_line(x3, y3, x1, y1, ch);
}
// --- Render & Display Routines ---
void render_canvas() {
    // Clear the board with background '_'
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            canvas[y][x] = '_';
        }
    }
    
    // Draw shapes in order of registration
    for (int i = 0; i < shape_count; i++) {
        Shape s = shapes[i];
        switch (s.type) {
            case SHAPE_LINE:
                draw_line(s.data.line.x1, s.data.line.y1, s.data.line.x2, s.data.line.y2, s.ch);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(s.data.rect.x, s.data.rect.y, s.data.rect.w, s.data.rect.h, s.ch);
                break;
            case SHAPE_CIRCLE:
                draw_circle(s.data.circle.cx, s.data.circle.cy, s.data.circle.r, s.ch);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(s.data.tri.x1, s.data.tri.y1, s.data.tri.x2, s.data.tri.y2, s.data.tri.x3, s.data.tri.y3, s.ch);
                break;
        }
    }
}
void display_canvas() {
    printf("\n");
    // Column Index (tens)
    printf("   ");
    for (int x = 0; x < WIDTH; x++) {
        if (x % 10 == 0) printf("%d", x / 10);
        else printf(" ");
    }
    printf("\n");
    
    // Column Index (ones)
    printf("   ");
    for (int x = 0; x < WIDTH; x++) {
        printf("%d", x % 10);
    }
    printf("\n");
    
    // Top Border
    printf("  +" BLUE);
    for (int x = 0; x < WIDTH; x++) printf("-");
    printf(RESET "+\n");
    
    // Canvas Body with Row Indices
    for (int y = 0; y < HEIGHT; y++) {
        printf("%2d" BLUE "|" RESET, y);
        for (int x = 0; x < WIDTH; x++) {
            char val = canvas[y][x];
            if (val == '_') {
                printf(BLUE "%c" RESET, val); // Render background subtly in blue
            } else {
                printf(GREEN BOLD "%c" RESET, val); // Render shapes in green/bold
            }
        }
        printf(BLUE "|" RESET "\n");
    }
    
    // Bottom Border
    printf("  +" BLUE);
    for (int x = 0; x < WIDTH; x++) printf("-");
    printf(RESET "+\n\n");
}
void list_shapes() {
    if (shape_count == 0) {
        printf(YELLOW "No active shapes on canvas.\n" RESET);
        return;
    }
    printf(CYAN BOLD "\n================== Active Shapes ==================\n" RESET);
    for (int i = 0; i < shape_count; i++) {
        Shape s = shapes[i];
        printf("ID: " BOLD "%d" RESET " | ", s.id);
        switch (s.type) {
            case SHAPE_LINE:
                printf("Line:      from (%d, %d) to (%d, %d) using '%c'\n", 
                       s.data.line.x1, s.data.line.y1, s.data.line.x2, s.data.line.y2, s.ch);
                break;
            case SHAPE_RECTANGLE:
                printf("Rectangle: top-left (%d, %d), size %dx%d using '%c'\n", 
                       s.data.rect.x, s.data.rect.y, s.data.rect.w, s.data.rect.h, s.ch);
                break;
            case SHAPE_CIRCLE:
                printf("Circle:    center (%d, %d), radius %d using '%c'\n", 
                       s.data.circle.cx, s.data.circle.cy, s.data.circle.r, s.ch);
                break;
            case SHAPE_TRIANGLE:
                printf("Triangle:  vertices (%d, %d), (%d, %d), (%d, %d) using '%c'\n", 
                       s.data.tri.x1, s.data.tri.y1, s.data.tri.x2, s.data.tri.y2, s.data.tri.x3, s.data.tri.y3, s.ch);
                break;
        }
    }
    printf(CYAN BOLD "===================================================\n\n" RESET);
}
// --- Menu Functions ---
void add_shape() {
    if (shape_count >= MAX_SHAPES) {
        printf(RED "Error: Maximum shape capacity reached.\n" RESET);
        return;
    }
    
    printf(CYAN BOLD "\n--- Add a Shape ---\n" RESET);
    printf("1. Line\n2. Rectangle\n3. Circle\n4. Triangle\n");
    int type = read_int_range("Select shape type (1-4): ", 1, 4);
    
    Shape s;
    s.id = next_id++;
    s.type = (ShapeType)type;
    
    switch (s.type) {
        case SHAPE_LINE:
            s.data.line.x1 = read_int_range("Enter X1 (0-79): ", 0, WIDTH - 1);
            s.data.line.y1 = read_int_range("Enter Y1 (0-19): ", 0, HEIGHT - 1);
            s.data.line.x2 = read_int_range("Enter X2 (0-79): ", 0, WIDTH - 1);
            s.data.line.y2 = read_int_range("Enter Y2 (0-19): ", 0, HEIGHT - 1);
            break;
        case SHAPE_RECTANGLE:
            s.data.rect.x = read_int_range("Enter top-left X (0-79): ", 0, WIDTH - 1);
            s.data.rect.y = read_int_range("Enter top-left Y (0-19): ", 0, HEIGHT - 1);
            s.data.rect.w = read_int_range("Enter width (1-80): ", 1, WIDTH);
            s.data.rect.h = read_int_range("Enter height (1-20): ", 1, HEIGHT);
            break;
        case SHAPE_CIRCLE:
            s.data.circle.cx = read_int_range("Enter center X (0-79): ", 0, WIDTH - 1);
            s.data.circle.cy = read_int_range("Enter center Y (0-19): ", 0, HEIGHT - 1);
            s.data.circle.r  = read_int_range("Enter radius (1-40): ", 1, 40);
            break;
        case SHAPE_TRIANGLE:
            s.data.tri.x1 = read_int_range("Enter vertex 1 X (0-79): ", 0, WIDTH - 1);
            s.data.tri.y1 = read_int_range("Enter vertex 1 Y (0-19): ", 0, HEIGHT - 1);
            s.data.tri.x2 = read_int_range("Enter vertex 2 X (0-79): ", 0, WIDTH - 1);
            s.data.tri.y2 = read_int_range("Enter vertex 2 Y (0-19): ", 0, HEIGHT - 1);
            s.data.tri.x3 = read_int_range("Enter vertex 3 X (0-79): ", 0, WIDTH - 1);
            s.data.tri.y3 = read_int_range("Enter vertex 3 Y (0-19): ", 0, HEIGHT - 1);
            break;
    }
    
    s.ch = read_char("Enter character to draw shape", '*');
    shapes[shape_count++] = s;
    printf(GREEN "Shape added successfully with ID %d!\n" RESET, s.id);
}
void delete_shape() {
    if (shape_count == 0) {
        printf(YELLOW "No shapes available to delete.\n" RESET);
        return;
    }
    
    list_shapes();
    int id = read_int_range("Enter shape ID to delete: ", 1, next_id - 1);
    
    int index = -1;
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].id == id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        printf(RED "Shape with ID %d not found.\n" RESET, id);
        return;
    }
    
    // Shift the remaining shapes
    for (int i = index; i < shape_count - 1; i++) {
        shapes[i] = shapes[i + 1];
    }
    shape_count--;
    printf(GREEN "Shape with ID %d deleted successfully.\n" RESET, id);
}
void modify_shape() {
    if (shape_count == 0) {
        printf(YELLOW "No shapes available to modify.\n" RESET);
        return;
    }
    
    list_shapes();
    int id = read_int_range("Enter shape ID to modify: ", 1, next_id - 1);
    
    int index = -1;
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].id == id) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        printf(RED "Shape with ID %d not found.\n" RESET, id);
        return;
    }
    
    Shape* s = &shapes[index];
    printf(CYAN BOLD "\n--- Modifying Shape [ID: %d] ---\n" RESET, s->id);
    
    switch (s->type) {
        case SHAPE_LINE:
            s->data.line.x1 = read_int_range("Enter new X1 (0-79): ", 0, WIDTH - 1);
            s->data.line.y1 = read_int_range("Enter new Y1 (0-19): ", 0, HEIGHT - 1);
            s->data.line.x2 = read_int_range("Enter new X2 (0-79): ", 0, WIDTH - 1);
            s->data.line.y2 = read_int_range("Enter new Y2 (0-19): ", 0, HEIGHT - 1);
            break;
        case SHAPE_RECTANGLE:
            s->data.rect.x = read_int_range("Enter new top-left X (0-79): ", 0, WIDTH - 1);
            s->data.rect.y = read_int_range("Enter new top-left Y (0-19): ", 0, HEIGHT - 1);
            s->data.rect.w = read_int_range("Enter new width (1-80): ", 1, WIDTH);
            s->data.rect.h = read_int_range("Enter new height (1-20): ", 1, HEIGHT);
            break;
        case SHAPE_CIRCLE:
            s->data.circle.cx = read_int_range("Enter new center X (0-79): ", 0, WIDTH - 1);
            s->data.circle.cy = read_int_range("Enter new center Y (0-19): ", 0, HEIGHT - 1);
            s->data.circle.r  = read_int_range("Enter new radius (1-40): ", 1, 40);
            break;
        case SHAPE_TRIANGLE:
            s->data.tri.x1 = read_int_range("Enter new vertex 1 X (0-79): ", 0, WIDTH - 1);
            s->data.tri.y1 = read_int_range("Enter new vertex 1 Y (0-19): ", 0, HEIGHT - 1);
            s->data.tri.x2 = read_int_range("Enter new vertex 2 X (0-79): ", 0, WIDTH - 1);
            s->data.tri.y2 = read_int_range("Enter new vertex 2 Y (0-19): ", 0, HEIGHT - 1);
            s->data.tri.x3 = read_int_range("Enter new vertex 3 X (0-79): ", 0, WIDTH - 1);
            s->data.tri.y3 = read_int_range("Enter new vertex 3 Y (0-19): ", 0, HEIGHT - 1);
            break;
    }
    s->ch = read_char("Enter character to draw shape", s->ch);
    printf(GREEN "Shape with ID %d modified successfully.\n" RESET, s->id);
}
void clear_canvas() {
    shape_count = 0;
    next_id = 1;
    printf(GREEN "Canvas cleared (all shapes deleted).\n" RESET);
}
int main() {
    printf(MAGENTA BOLD "===============================================\n" RESET);
    printf(MAGENTA BOLD "    Welcome to the 2D ASCII Graphics Editor     \n" RESET);
    printf(MAGENTA BOLD "===============================================\n" RESET);
    
    while (1) {
        render_canvas();
        display_canvas();
        
        printf(BOLD "Menu Options:\n" RESET);
        printf("  1. Add Shape\n");
        printf("  2. Delete Shape\n");
        printf("  3. Modify Shape\n");
        printf("  4. Clear Canvas\n");
        printf("  5. Exit\n");
        
        int choice = read_int_range("Enter your choice (1-5): ", 1, 5);
        
        switch (choice) {
            case 1:
                add_shape();
                break;
            case 2:
                delete_shape();
                break;
            case 3:
                modify_shape();
                break;
            case 4:
                clear_canvas();
                break;
            case 5:
                printf(MAGENTA BOLD "\nThank you for using the 2D Graphics Editor! Goodbye.\n" RESET);
                return 0;
        }
    }
    return 0;
}
