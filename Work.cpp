#include "Work.h"
#include <hpdf.h>
#include <csetjmp>
#include <iostream>
#include <map>
#include <iomanip>
// Global error handler for libharu
jmp_buf env;

#ifdef HPDF_DLL
void __stdcall
#else
void
#endif
error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data) {
    fprintf(stderr, "HPDF ERROR: error_no=0x%04X, detail_no=%u\n", (HPDF_UINT)error_no, (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

Work::Work() {
    employees_per_shift.resize(DAYS, std::vector<int>(NUM_OF_SHIFTS, 4));
    schedule.resize(DAYS, std::vector<std::vector<int>>(NUM_OF_SHIFTS));
}

void Work::setEmployeesPerShift(int day, int shift, int num_of_employees) {
    if (day >= 0 && day < DAYS && shift >= 0 && shift < NUM_OF_SHIFTS) {
        employees_per_shift[day][shift] = num_of_employees;
    }
}

void Work::addEmployee(const Employee& e) { employees.push_back(e); }
const std::vector<Employee>& Work::getEmployees() const { return employees; }
const std::vector<std::vector<std::vector<int>>>& Work::getSchedule() const { return schedule; }

bool Work::createSchedule() {
    for (auto& day_schedule : schedule) {
        for (auto& shift_schedule : day_schedule) {
            shift_schedule.clear();
        }
    }
    return solve(0, 0);
}

bool Work::solve(int day, int shift) {
    if (day == DAYS) {
        return true; // Base case: All days and shifts are scheduled.
    }

    // Calculate the next day and shift.
    int next_day = day;
    int next_shift = shift + 1;
    if (next_shift >= NUM_OF_SHIFTS) {
        next_shift = 0;
        next_day++;
    }

    // Get employees already working on the current day to avoid double-booking.
    std::set<int> working_today_ids;
    if (shift > 0) { // Only need to check previous shifts on the same day.
        for (int s = 0; s < shift; ++s) {
            for (int id : schedule[day][s]) {
                working_today_ids.insert(id);
            }
        }
    }

    // Start the backtracking process for the current shift.
    if (solveForShift(day, shift, 0, working_today_ids)) {
        // If the current shift is solved, move to the next one.
        return solve(next_day, next_shift);
    }

    return false; // Could not find a solution.
}

bool Work::solveForShift(int day, int shift, int employee_idx, std::set<int>& working_today) {
    // If the current shift is full, we've succeeded for this shift.
    if (schedule[day][shift].size() >= employees_per_shift[day][shift]) {
        return true;
    }

    // If we've tried all employees and the shift is not full, backtrack.
    if (employee_idx >= employees.size()) {
        return false;
    }

    // Try to assign employees starting from employee_idx.
    for (size_t i = employee_idx; i < employees.size(); ++i) {
        const auto& emp = employees[i];
        if (working_today.find(emp.getID()) == working_today.end() && emp.canWork(day, shift)) {
            // Assign employee
            schedule[day][shift].push_back(emp.getID());
            working_today.insert(emp.getID());

            // Recurse to fill the next spot in the *same* shift.
            if (solveForShift(day, shift, i + 1, working_today)) {
                return true; // Found a valid assignment for the rest of the shift.
            }

            // Backtrack
            working_today.erase(emp.getID());
            schedule[day][shift].pop_back();
        }
    }

    // No valid assignment found from this state.
    return false;
}
void printScheduleTable(const Work& work_schedule) {
    std::cout << "\nSchedule Table:\n";
    std::cout << "Day\\Shift |";
    for (int s = 0; s < Work::NUM_OF_SHIFTS; ++s) std::cout << " Shift " << s << " |";
    std::cout << "\n";
    for (int s = 0; s < Work::NUM_OF_SHIFTS + 1; ++s) std::cout << "-----------";
    std::cout << "\n";

    std::map<int, std::string> employee_names;
    for (const auto& emp : work_schedule.getEmployees())
        employee_names[emp.getID()] = emp.getName();

    for (int day = 0; day < Work::DAYS; ++day) {
        std::cout << "Day " << day << "      |";
        for (int shift = 0; shift < Work::NUM_OF_SHIFTS; ++shift) {
            std::string employees_str;
            const auto& scheduled_ids = work_schedule.getSchedule()[day][shift];
            if (!scheduled_ids.empty()) {
                employees_str += employee_names[scheduled_ids[0]];
                for (size_t i = 1; i < scheduled_ids.size(); ++i) {
                    employees_str += ", " + employee_names[scheduled_ids[i]];
                }
            }
            std::cout << " " << (employees_str.empty() ? "-" : employees_str) << " |";
        }
        std::cout << "\n";
    }
}

std::vector<std::string> wrapText(const std::string& text, HPDF_Font font, float font_size, float max_width, HPDF_Page page) {
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string word, line;
    while (iss >> word) {
        std::string testLine = line.empty() ? word : line + " " + word;
        float width = HPDF_Page_TextWidth(page, testLine.c_str());
        if (width > max_width && !line.empty()) {
            lines.push_back(line);
            line = word;
        } else {
            line = testLine;
        }
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

int Work::createPDF(const Work& work_schedule) {
    std::vector<std::string> days = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"};
    std::vector<std::string> shifts = {"Morning Shift", "Night Shift"};
    std::map<int, std::string> employee_names;

    for (const auto& emp : work_schedule.getEmployees())
        employee_names[emp.getID()] = emp.getName();

    HPDF_Doc pdf = HPDF_New(error_handler, NULL);
    if (!pdf) {
        std::cerr << "Failed to create PDF document.\n";
        return 1;
    }
    if (setjmp(env)) {
        HPDF_Free(pdf);
        std::cerr << "An error occurred during PDF generation.\n";
        return 1;
    }

    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", nullptr);
    HPDF_Font font_bold = HPDF_GetFont(pdf, "Helvetica-Bold", nullptr);

    float page_height = HPDF_Page_GetHeight(page);
    float page_width = HPDF_Page_GetWidth(page);
    float margin = 50;
    float table_top = page_height - margin - 30;
    float col_width_day = 90;
    float col_width_shift = 180;
    float header_height = 30;
    float base_row_height = 25;

    // Title
    HPDF_Page_SetFontAndSize(page, font_bold, 16);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, margin, page_height - margin, "Work Schedule");
    HPDF_Page_EndText(page);

    // Header background
    HPDF_Page_SetRGBFill(page, 0.2, 0.4, 0.7);
    HPDF_Page_Rectangle(page, margin, table_top - header_height, 
                        col_width_day + Work::NUM_OF_SHIFTS * col_width_shift, header_height);
    HPDF_Page_Fill(page);

    // Header text
    HPDF_Page_SetRGBFill(page, 1.0, 1.0, 1.0);
    HPDF_Page_SetFontAndSize(page, font_bold, 11);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, margin + 10, table_top - header_height + 10, "Day/Shift");
    HPDF_Page_EndText(page);

    for (int s = 0; s < Work::NUM_OF_SHIFTS; ++s) {
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, margin + col_width_day + s * col_width_shift + 10, 
                         table_top - header_height + 10, shifts[s].c_str());
        HPDF_Page_EndText(page);
    }

    HPDF_Page_SetRGBFill(page, 0.0, 0.0, 0.0);

    // Calculate dynamic row heights
    std::vector<float> row_heights(Work::DAYS);
    for (int day = 0; day < Work::DAYS; ++day) {
        float max_row_height = base_row_height;
        for (int shift = 0; shift < Work::NUM_OF_SHIFTS; ++shift) {
            std::string employees_str;
            const auto& scheduled_ids = work_schedule.getSchedule()[day][shift];
            if (!scheduled_ids.empty()) {
                employees_str += employee_names[scheduled_ids[0]];
                for (size_t i = 1; i < scheduled_ids.size(); ++i)
                    employees_str += ", " + employee_names[scheduled_ids[i]];
            } else {
                employees_str = "-";
            }

            auto lines = wrapText(employees_str, font, 9, col_width_shift - 20, page);
            float needed_height = lines.size() * 12 + 8;
            if (needed_height > max_row_height)
                max_row_height = needed_height;
        }
        row_heights[day] = max_row_height;
    }

    // Draw table rows
    float current_y = table_top - header_height;
    for (int day = 0; day < Work::DAYS; ++day) {
        float this_row_height = row_heights[day];
        current_y -= this_row_height;
        float y_text = current_y + this_row_height - 15;

        // Alternating background
        if (day % 2 == 0) {
            HPDF_Page_SetRGBFill(page, 0.95, 0.95, 0.95);
            HPDF_Page_Rectangle(page, margin, current_y, 
                                col_width_day + Work::NUM_OF_SHIFTS * col_width_shift, this_row_height);
            HPDF_Page_Fill(page);
            HPDF_Page_SetRGBFill(page, 0.0, 0.0, 0.0);
        }

        // Day name
        HPDF_Page_SetFontAndSize(page, font_bold, 10);
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, margin + 10, y_text, days[day].c_str());
        HPDF_Page_EndText(page);

        // Shift data with wrapping
        HPDF_Page_SetFontAndSize(page, font, 9);
        for (int shift = 0; shift < Work::NUM_OF_SHIFTS; ++shift) {
            std::string employees_str;
            const auto& scheduled_ids = work_schedule.getSchedule()[day][shift];
            if (!scheduled_ids.empty()) {
                employees_str += employee_names[scheduled_ids[0]];
                for (size_t i = 1; i < scheduled_ids.size(); ++i)
                    employees_str += ", " + employee_names[scheduled_ids[i]];
            } else {
                employees_str = "-";
            }

            auto lines = wrapText(employees_str, font, 9, col_width_shift - 20, page);
            float line_y = y_text;
            for (const auto& line : lines) {
                HPDF_Page_BeginText(page);
                HPDF_Page_TextOut(page, margin + col_width_day + shift * col_width_shift + 10, line_y, line.c_str());
                HPDF_Page_EndText(page);
                line_y -= 12;
            }
        }
    }

    HPDF_Page_SetLineWidth(page, 1.0);
    HPDF_Page_SetRGBStroke(page, 0.2, 0.2, 0.2);

    float total_table_height = header_height;
    for (float h : row_heights) total_table_height += h;
    HPDF_Page_Rectangle(page, margin, table_top - total_table_height, 
                        col_width_day + Work::NUM_OF_SHIFTS * col_width_shift, total_table_height);
    HPDF_Page_Stroke(page);

    float border_y = table_top - header_height;
    HPDF_Page_SetLineWidth(page, 0.5);
    HPDF_Page_SetRGBStroke(page, 0.6, 0.6, 0.6);
    for (int day = 0; day < Work::DAYS; ++day) {
        border_y -= row_heights[day];
        HPDF_Page_MoveTo(page, margin, border_y);
        HPDF_Page_LineTo(page, margin + col_width_day + Work::NUM_OF_SHIFTS * col_width_shift, border_y);
        HPDF_Page_Stroke(page);
    }

    HPDF_Page_SetLineWidth(page, 0.5);
    for (int s = 0; s <= Work::NUM_OF_SHIFTS; ++s) {
        float x = margin + col_width_day + s * col_width_shift;
        HPDF_Page_MoveTo(page, x, table_top);
        HPDF_Page_LineTo(page, x, table_top - total_table_height);
        HPDF_Page_Stroke(page);
    }

    HPDF_SaveToFile(pdf, "schedule.pdf");
    HPDF_Free(pdf);
    return 0;
}
