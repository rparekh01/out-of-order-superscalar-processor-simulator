#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <math.h>
#include <bitset>
#include <algorithm>
#include <math.h>
#include <iomanip>

struct ROB
{
    int op_type = 0;
    int dest_reg = 0;
    int src_reg1 = 0;
    int src_reg2 = 0;

    std::string src1_flag = "ready";
    std::string src2_flag = "ready";
    std::string state = "if"; // IF, ID, IS, EX, WB
    unsigned int tag = 0;
    //unsigned int id = 0;
    unsigned int src1_tag = 0;
    unsigned int src2_tag = 0;
    unsigned int IF_duration = 0;
    unsigned int ID_duration = 0;
    unsigned int IS_duration = 0;
    unsigned int EX_duration = 1;
    unsigned int WB_duration = 0; // constant
    unsigned int IF_cycle = 0;
    unsigned int ID_cycle = 0;
    unsigned int IS_cycle = 0;
    unsigned int EX_cycle = 0;
    unsigned int WB_cycle = 0;

    unsigned int ex_stall = 0;
};

std::vector<ROB> instruction_data;

std::vector<ROB> dispatch_queue;
std::vector<ROB> schedule_queue;
std::vector<ROB> ex_queue;
std::vector<ROB> wb_queue;
unsigned int cycle_count = 0;
unsigned int tag_counter = 0;
unsigned int schedule_size = 0;
unsigned int peak_rate = 0;

std::vector<std::string> file_content;
std::string trace_file = "";

void execute(unsigned int n_size, unsigned int s_size);
void issue(unsigned int n_size, unsigned int s_size);
void dispatch(unsigned int n_size, unsigned int s_size);
void fetch(unsigned int n_size, unsigned int s_size);
bool sort_tag(const ROB &x, const ROB &y);
int main(int argc, char *argv[])
{
    char *pCh;
    schedule_size = strtoul(argv[1], &pCh, 10);
    peak_rate = strtoul(argv[2], &pCh, 10);
    trace_file = argv[argc - 1];

    // resize lists
    //dispatch_queue.resize(peak_rate * 2);
    //schedule_queue.resize(schedule_size);
    //ex_queue.resize(peak_rate + 1);

    //std::cout << dispatch_queue.size() << std::endl;
    //std::cout << ex_queue.size() << std::endl;
    //std::cout << schedule_queue.size() << std::endl;

    std::string token = "";
    std::vector<std::string> input;
    unsigned int hexAd = 0;
    std::fstream myfile(trace_file);
    std::string line = "";
    unsigned int idx_bits = 0;
    unsigned int fileLen = 1;

    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            if (line.empty())
            {
                continue;
            }
            file_content.push_back(line);
        }
        myfile.close();
    }
    else
    {
        std::cout << "**" << trace_file << " doesn't exist.**" << std::endl;
        std::cout << "**Please make sure that the trace files are in the same directory as sim executable.**" << std::endl;
    }

    instruction_data.resize(file_content.size());

    for (int i = 0; i < file_content.size(); i++)
    {
        std::istringstream stm(file_content[i]);
        input.clear();
        while (stm >> token)
        {

            input.push_back(token);
        }
        instruction_data[i].tag = i;
        std::stringstream(input[1]) >> instruction_data[i].op_type;
        std::stringstream(input[2]) >> instruction_data[i].dest_reg;
        std::stringstream(input[3]) >> instruction_data[i].src_reg1;
        std::stringstream(input[4]) >> instruction_data[i].src_reg2;
        if (instruction_data[i].op_type == 0)
        {
            instruction_data[i].ex_stall = 1;
        }
        else if (instruction_data[i].op_type == 1)
        {
            instruction_data[i].ex_stall = 2;
        }
        else if (instruction_data[i].op_type == 2)
        {
            instruction_data[i].ex_stall = 5;
        }
    }
    int total_instruction = instruction_data.size();
    /*for (int i = 0; i < instruction_data.size(); i++)
    {
        //std::cout << instruction_data[i].op_type << std::endl;
        //std::cout << instruction_data[i].dest_reg << std::endl;
        //std::cout << instruction_data[i].src_reg1 << std::endl;
        //std::cout << instruction_data[i].src_reg2 << std::endl;
        std::cout << instruction_data[i].tag << std::endl;

    }*/

    /*for (int i = 0; i < 4; i++)
    {
        std::cout << "tag: " << instruction_data[i].tag << "fu{" << instruction_data[i].op_type << "} "
                  << "src{" << instruction_data[i].src_reg1 << "," << instruction_data[i].src_reg2 << "}" << std::endl;
    }*/

    while (wb_queue.size() < total_instruction)
    {
        execute(peak_rate, schedule_size);
        issue(peak_rate, schedule_size);
        dispatch(peak_rate, schedule_size);
        fetch(peak_rate, schedule_size);

        //dispatch(peak_rate, schedule_size);
        //issue(peak_rate, schedule_size);
        //execute(peak_rate, schedule_size);

        //std::cout << cycle_count << std::endl;
        // incremeant cycle
        //std::cout << "wb_queue queue size: " << wb_queue.size() << std::endl;
        //std::cout << "dispatch queue size: " << dispatch_queue.size() << std::endl;
        //std::cout << "schedule queue size: " << schedule_queue.size() << std::endl;
        //std::cout << "ex queue size: " << ex_queue.size() << std::endl;
        /*for(int i = 0; i < dispatch_queue.size(); i++){
            std::cout << "dispatch queue state: " << dispatch_queue[i].state << std::endl;
        }*/

        //std::cout << "is_cycle : " << cycle_count << std::endl;
        cycle_count++;
        //std::cout << "schedule queue size = " << schedule_queue.size() << std::endl;
        /*if (cycle_count == 400)
        {
            std::cout << "schedule queue..." << std::endl;
            for (int i = 0; i < schedule_queue.size(); i++)
            {
                std::cout << schedule_queue[i].tag << " " << schedule_queue[i].src_reg1 << " " << schedule_queue[i].src_reg2 << std::endl;

                //std::cout << "tag: " << schedule_queue[i].tag << std::endl;
                std::cout << " src1 flag: " << schedule_queue[i].src1_flag << std::endl;
                std::cout << " src2 flag: " << schedule_queue[i].src2_flag << std::endl;
                std::cout << " src1 tag: " << schedule_queue[i].src1_tag << std::endl;
                std::cout << " src2 tag: " << schedule_queue[i].src2_tag << std::endl;
            }
        }
        if (cycle_count == 400)
        {
            break;
        }*/

    }
    /*for (int i = 0; i < schedule_queue.size(); i++)
    {
        std::cout << schedule_queue[i].tag << " state " << schedule_queue[i].state << std::endl;
        //std::cout << "tag: " << schedule_queue[i].tag << std::endl;
        std::cout << " src1 flag: " << schedule_queue[i].src1_flag << std::endl;
        std::cout << " src2 flag: " << schedule_queue[i].src2_flag << std::endl;
    }*/
    //std::cout << "ex queue..." << std::endl;
    /*for (int i = 0; i < ex_queue.size(); i++)
    {
        std::cout << ex_queue[i].tag << " state " << ex_queue[i].state << std::endl;
        //std::cout << "tag: " << schedule_queue[i].tag << std::endl;
        std::cout << " src1 tag: " << ex_queue[i].src1_tag << std::endl;
        std::cout << " src2 tag: " << ex_queue[i].src2_tag << std::endl;
    }
    std::cout << "ex size " << ex_queue.size() << std::endl;
    */
    //std::cout << "schedule queue size = " << schedule_queue.size() << std::endl;
    //std::cout << "schedule queue is cycle = " << schedule_queue[0].IS_cycle << std::endl;
    //std::cout << "schedule queue is cycle = " << schedule_queue[1].IS_cycle << std::endl;
    //std::cout << "schedule queue is cycle = " << schedule_queue[2].IS_cycle << std::endl;
    //std::cout << "schedule queue is cycle = " << schedule_queue[3].IS_cycle << std::endl;

    std::sort(wb_queue.begin(), wb_queue.end(), sort_tag);
    for (int i = 0; i < wb_queue.size(); i++)
    {
        // 0 fu{0} src{29,14} dst{-1} IF{0,1} ID{1,1} IS{2,1} EX{3,1} WB{4,1}
        std::cout << wb_queue[i].tag << "fu{" << wb_queue[i].op_type << "} "
                  << "src{" << wb_queue[i].src_reg1 << "," << wb_queue[i].src_reg2 << "} "
                  << "dst{" << wb_queue[i].dest_reg << "} "
                  << "IF{" << wb_queue[i].IF_cycle << "," << wb_queue[i].IF_duration << "} "
                  << "ID{" << wb_queue[i].ID_cycle << "," << wb_queue[i].ID_duration << "} "
                  << "IS{" << wb_queue[i].IS_cycle << "," << wb_queue[i].IS_duration << "} "
                  << "EX{" << wb_queue[i].EX_cycle << "," << wb_queue[i].EX_duration << "} "
                  << "WB{" << wb_queue[i].WB_cycle << "," << wb_queue[i].WB_duration << "} "
                  //<< wb_queue[i].src1_tag << " " << wb_queue[i].src2_tag
                  << std::endl;
    }
    //std::cout << schedule_queue[7].tag << std::endl;
    //std::cout << schedule_queue[7].src1_tag << "  " << schedule_queue[7].src2_tag << std::endl;
}

void execute(unsigned int n_size, unsigned int s_size)
{
    for (int i = 0; i < ex_queue.size(); i++)
    {
        if (ex_queue[i].state == "ex")
        {
            if (ex_queue[i].EX_duration == ex_queue[i].ex_stall)
            {

                ex_queue[i].state = "wb";
                ex_queue[i].WB_cycle = cycle_count;
                ex_queue[i].WB_duration++;

                // store instruction details for output
                wb_queue.push_back(ex_queue[i]);

                // ex_queue space
            }
            else
            {
                ex_queue[i].EX_duration++;
            }
        }
    }

    for (unsigned int i = 0; i < ex_queue.size();)
    {
        if (ex_queue[i].state == "wb")
        {
            ex_queue.erase(ex_queue.begin() + i);
        }

        else
            i++;
    }
    std::sort(wb_queue.begin(), wb_queue.end(), sort_tag);

    for (int i = 0; i < schedule_queue.size(); i++)
    {
        /*
        if (cycle_count == 13)
        {

            std::cout << schedule_queue[i].tag << " " << schedule_queue[i].src_reg1 << " " << schedule_queue[i].src_reg2 << std::endl;
        }
        */

        for (int j = 0; j < wb_queue.size(); j++)
        {
            /*if (cycle_count == 13)
            {

                std::cout << "wb queue tag: " << wb_queue[i].tag << std::endl;
                std::cout << "sched queue tags" << std::endl;
            }*/
            if (wb_queue[j].tag == schedule_queue[i].src1_tag)
            {
                /*if (cycle_count == 13)
                {
                    std::cout << wb_queue[j].tag << " = " << schedule_queue[i].tag << std::endl;
                }*/
                schedule_queue[i].src1_flag = "ready";
            }
            if (wb_queue[j].tag == schedule_queue[i].src2_tag)
            {
                schedule_queue[i].src2_flag = "ready";
            }
            /*if (schedule_queue[j].dest_reg == wb_queue[i].dest_reg)
            {
                break;
            }*/
        }
    }
}

void issue(unsigned int n_size, unsigned int s_size)
{
    unsigned int fn_count = 0;
    for (int i = 0; i < schedule_queue.size(); i++)
    {
        if (schedule_queue[i].state == "is")
        {
            schedule_queue[i].IS_duration++;
            //check if both regs are ready
            if (schedule_queue[i].src1_flag == "ready" & schedule_queue[i].src2_flag == "ready")
            {

                // push the updated instruction to ex queue till (n_size+1)
                if (fn_count < (n_size + 1))
                {
                    // update state since both src reg are ready
                    schedule_queue[i].state = "ex";
                    schedule_queue[i].EX_cycle = cycle_count;
                    //schedule_queue[i].EX_duration++;
                    ex_queue.push_back(schedule_queue[i]);
                    fn_count++;
                    //free schedule_queue space
                    //schedule_queue.erase(schedule_queue.begin() + i);
                }
            }
            /*else if (schedule_queue[i].src1_flag != "ready" | schedule_queue[i].src2_flag != "ready")
            {
                schedule_queue[i].IS_duration++; // increment stall in IS state
            }*/
        }
    }

    for (unsigned int i = 0; i < schedule_queue.size();)
    {
        if (schedule_queue[i].state == "ex")
            schedule_queue.erase(schedule_queue.begin() + i);
        else
            i++;
    }

    /*for (int i = 0; i < schedule_queue.size(); i++)
    {
        if (schedule_queue[i].state == "ex")
        {
            schedule_queue.erase(schedule_queue.begin() + i);
        }
    }*/
}

// fetch instruction to dispatch queue and change state to IF
void fetch(unsigned int n_size, unsigned int s_size)
{
    for (int i = 0; i < n_size; i++)
    {
        if (dispatch_queue.size() < (n_size * 2))
        {
            if (instruction_data.empty())
            {
                break;
            }
            // push to dispatch queue
            instruction_data[0].IF_cycle = cycle_count; // capture cycle count at state change to IF
            instruction_data[0].IF_duration++;
            dispatch_queue.push_back(instruction_data[0]);

            // change empty state to IF state
            //dispatch_queue[i].state = "if";

            instruction_data.erase(instruction_data.begin());
        }
    }
}
void dispatch(unsigned int n_size, unsigned int s_size)
{
    for (int i = 0; i < dispatch_queue.size(); i++)
    {
        if (dispatch_queue[i].state == "id")
        {
            if (schedule_queue.size() < s_size)
            {
                dispatch_queue[i].state = "is"; // change state to IS
                dispatch_queue[i].IS_cycle = cycle_count;
                /*if (dispatch_queue[i].dest_reg != -1)
                {
                    dispatch_queue[i].tag = tag_counter; // add tag for dest reg

                    tag_counter++;
                }*/

                /*if (dispatch_queue[i].src_reg1 != -1 && dispatch_queue[i].src1_tag == 0)
                {
                    if (!ex_queue.empty())
                    {
                        std::sort(ex_queue.begin(), ex_queue.end(), sort_tag);
                        for (int j = 0; j < ex_queue.size(); j++)
                        {
                            if (ex_queue[j].dest_reg == dispatch_queue[i].src_reg1 && ex_queue[j].dest_reg != -1)
                            {
                                dispatch_queue[i].src1_tag = ex_queue[j].tag;
                                dispatch_queue[i].src1_flag = "not";
                            }
                        }
                    }
                }
                if (dispatch_queue[i].src_reg2 != -1 && dispatch_queue[i].src2_tag == 0)
                {
                    if (!ex_queue.empty())
                    {
                        std::sort(ex_queue.begin(), ex_queue.end(), sort_tag);
                        for (int j = 0; j < ex_queue.size(); j++)
                        {

                            if (ex_queue[j].dest_reg == dispatch_queue[i].src_reg2 && ex_queue[j].dest_reg != -1)
                            {
                                dispatch_queue[i].src2_tag = ex_queue[j].tag;
                                dispatch_queue[i].src2_flag = "not";
                            }
                        }
                    }
                }*/
                if (!schedule_queue.empty())
                {
                    for (int j = 0; j < schedule_queue.size(); j++)
                    {
                        if (dispatch_queue[i].src_reg1 == schedule_queue[j].dest_reg && schedule_queue[j].dest_reg != -1)
                        {
                            if (dispatch_queue[i].src1_tag == 0)
                            {
                                dispatch_queue[i].src1_tag = schedule_queue[j].tag;
                                dispatch_queue[i].src1_flag = "not";
                            }
                            else if (dispatch_queue[i].src1_tag != 0 && dispatch_queue[i].src1_tag < schedule_queue[j].tag)
                            {
                                dispatch_queue[i].src1_tag = schedule_queue[j].tag;
                                dispatch_queue[i].src1_flag = "not";
                            }
                        }
                        if (dispatch_queue[i].src_reg2 == schedule_queue[j].dest_reg && schedule_queue[j].dest_reg != -1)
                        {
                            if (dispatch_queue[i].src2_tag == 0)
                            {
                                dispatch_queue[i].src2_tag = schedule_queue[j].tag;
                                dispatch_queue[i].src2_flag = "not";
                            }
                            else if (dispatch_queue[i].src2_tag != 0 && dispatch_queue[i].src2_tag < schedule_queue[j].tag)
                            {
                                dispatch_queue[i].src2_tag = schedule_queue[j].tag;
                                dispatch_queue[i].src2_flag = "not";
                            }
                        }
                    }
                }

                if (!ex_queue.empty())
                {
                    std::sort(ex_queue.begin(), ex_queue.end(), sort_tag);
                    for (int j = 0; j < ex_queue.size(); j++)
                    {
                        /*if (dispatch_queue[i].tag == 496)
                        {
                            std::cout << ex_queue[j].tag << std::endl;
                        }*/

                        if (dispatch_queue[i].tag == 496 && ex_queue[j].tag == 492)
                        {

                            /*if (dispatch_queue[i].src1_tag < ex_queue[j].tag)
                            {
                                std::cout << dispatch_queue[i].tag << " "
                                          << dispatch_queue[i].src1_tag << " " << ex_queue[j].tag << std::endl;
                            }*/
                        }

                        if (ex_queue[j].dest_reg == dispatch_queue[i].src_reg1 && ex_queue[j].dest_reg != -1)
                        {

                            if (dispatch_queue[i].src1_tag == 0)
                            {
                                dispatch_queue[i].src1_tag = ex_queue[j].tag;
                                dispatch_queue[i].src1_flag = "not";
                            }
                            else if (dispatch_queue[i].src1_tag < ex_queue[j].tag)
                            {
                                dispatch_queue[i].src1_tag = ex_queue[j].tag;
                                dispatch_queue[i].src1_flag = "not";
                            }
                        }
                        if (ex_queue[j].dest_reg == dispatch_queue[i].src_reg2 && ex_queue[j].dest_reg != -1)
                        {
                            if (dispatch_queue[i].src2_tag == 0)
                            {
                                dispatch_queue[i].src2_tag = ex_queue[j].tag;
                                dispatch_queue[i].src2_flag = "not";
                            }
                            else if (dispatch_queue[i].src2_tag < ex_queue[j].tag)
                            {
                                dispatch_queue[i].src2_tag = ex_queue[j].tag;
                                dispatch_queue[i].src2_flag = "not";
                            }
                        }
                    }
                }
                std::sort(wb_queue.begin(), wb_queue.end(), sort_tag);
                for (int j = 0; j < wb_queue.size(); j++)
                {
                    if (wb_queue[j].dest_reg == dispatch_queue[i].src_reg1 && wb_queue[j].dest_reg != -1)
                    {

                        if (dispatch_queue[i].src1_tag == 0)
                        {
                            dispatch_queue[i].src1_tag = wb_queue[j].tag;
                            dispatch_queue[i].src1_flag = "ready";
                        }
                        else if (dispatch_queue[i].src1_tag < wb_queue[j].tag)
                        {
                            dispatch_queue[i].src1_tag = wb_queue[j].tag;
                            dispatch_queue[i].src1_flag = "ready";
                        }
                    }
                    if (wb_queue[j].dest_reg == dispatch_queue[i].src_reg2 && wb_queue[j].dest_reg != -1)
                    {
                        if (dispatch_queue[i].src2_tag == 0)
                        {
                            dispatch_queue[i].src2_tag = wb_queue[j].tag;
                            dispatch_queue[i].src2_flag = "ready";
                        }
                        else if (dispatch_queue[i].src2_tag < wb_queue[j].tag)
                        {
                            dispatch_queue[i].src2_tag = wb_queue[j].tag;
                            dispatch_queue[i].src2_flag = "ready";
                        }
                    }
                }  
                // capture cycle count at state change
                schedule_queue.push_back(dispatch_queue[i]);
                // dispatch_queue.erase(dispatch_queue.begin() + i); // free dispatch queue space
            }
            else if (schedule_queue.size() == s_size) // if schedue queue full
            {
                //increment stall in ID state
                dispatch_queue[i].ID_duration++;
            }
        }
        else if (dispatch_queue[i].state == "if") // unconditional IF to ID transition
        {
            dispatch_queue[i].state = "id";
            dispatch_queue[i].ID_duration++;

            dispatch_queue[i].ID_cycle = cycle_count; // capture cycle count at state change to ID
        }
    }

    for (unsigned int i = 0; i < dispatch_queue.size();)
    {
        if (dispatch_queue[i].state == "is")
            dispatch_queue.erase(dispatch_queue.begin() + i);
        else
            i++;
    }
    /*for (int i = 0; i < schedule_queue.size(); i++)
    {
        if(schedule_queue[i].dest_reg == -1){
                continue;
            }
        for (int j = 1; j < schedule_queue.size(); j++)
        {
            

            if (schedule_queue[i].dest_reg == schedule_queue[j].src_reg1 && schedule_queue[j].id > schedule_queue[i].id)
            {
                

                schedule_queue[j].src1_flag = "not";
                schedule_queue[j].src1_tag = schedule_queue[i].tag;
            }

            if (schedule_queue[i].dest_reg == schedule_queue[j].src_reg2 && schedule_queue[j].id > schedule_queue[i].id)
            {

                schedule_queue[j].src2_flag = "not";
                schedule_queue[j].src2_tag = schedule_queue[i].tag;
            }
            if (schedule_queue[i].dest_reg == schedule_queue[j].dest_reg)
            {
                break;
            }
        }
    }*/
}

bool sort_tag(const ROB &x, const ROB &y)
{
    return x.tag < y.tag;
}