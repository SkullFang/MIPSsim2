/*
 * 程序主要分为三个部分
第一步disassembly把处理好的vector放入弄成指令形式的
1.IF
第一步，看Unit[0]是不是空的，如果不是空的。不是空就进去看每条指令所用
寄存器的状态，这主要存的是分支指令。如果寄存器状态允许去执行就这条指令就进去到Unit[1]
第二步，看Unit[1]是不是空，如果不是空。就进去看看。一些跳转指令该执行了。
2.WB
第一步，看看post-alu里面有没有东西。如果有的话就去执行
第二不，看看post-mem里面有没有东西，如果有的话就去执行。这里一定是LW
3.MEM
一步，看看premem里买呢有啥，如果是LW就把它放到post-MEM,如果是LW就去执行掉
4.ALU
一步，看看pre-alu里面有啥。如果是LW，SW就放入pre-mem,如果是算数类的就放在post-alu
5。ISSUE
第一步，看看能不能ISSUE，如果可以的话。找到能issue的指令。
第二步，遍历能issue的指令，确定能ISSUE的次数，进行顺序ISSUE
6. FETCH
第一步，看第一条能不能FETCH。
第二步，看看第二条能不能FETCH
 */
#include <iostream>
#include <vector>
#include <fstream>
#include <math.h>
#include <iomanip>
#include <map>
using namespace std;

void pipLine(vector<string>& va);
void simulation(vector<string>& va,int start);
void output_simulation(int cycle);

vector<int> reg(32,0);  //声明32个寄存器，初始值为0
vector<pair<int,int> > my_data;// add,value > >这是地址和data的映射
map<int,int> my_map;// 地址和指令的映射

// new add
vector<int> reg_state(32,1); //寄存器的状态
vector<string> unit(2,""); //0-wait , 1-execute  IF的两个单元
vector<string> pre_issue_queue(4,""); //pre_issue_queue队列
int sum_pre_issue_queue = 0; //pre_issue_queue队列的使用个数
vector<string> pre_alu_queue(2,""); //pre_lu_queue 队列
int sum_pre_alu_queue = 0; //alu队列的使用个数
string pre_mem_queue = ""; //pre_mem
string post_mem_queue = "";//post_mem
string post_alu_queue = "";//post_alu
vector<string> sum_temp(5,"");
vector<string> sum_temp2(9,"");
vector<int> reg_state2(32,1);//寄存器的状态，之所以还有一个是为了程序运行的时候有备份，备份做为判断条件


int main(int argc, const char * argv[]) {
    vector<string> instruction;
    ifstream my_in(argv[1]);
    for(string s;my_in >> s;) {
        instruction.push_back(s);
    }
    //文件读入工作
    pipLine(instruction);
    return 0;
}

void pipLine(vector<string>& va) {
//    int i;
    int data_start_flag=0;
    int data_start=0;
    for(int i=0;i<va.size();i++){//一行一行的读
        int add = 128 + i * 4;//地址4个4个往上加
        va[i] = va[i] + '\t' + to_string(add) + '\t';// 指令+地址
        my_map[add] = i;
        string start_0 = va[i].substr(0,3);//指令的前三位，用与分类
        //如果是数字
        if(data_start_flag==1){// BERAK没有执行完这个就开
            int add = 128 + i * 4;
            va[i] = va[i] + '\t' + to_string(add) + '\t';
            pair<int,int> temp_data;
            temp_data.first = add; //地址
            string str = va[i].substr(0,32);
            if(str[0] == '1') {
                va[i] += '-';
                for(int j = 1;j<str.size();++j) {
                    if(str[j] == '1')
                        str[j] = '0';
                    else
                        str[j] = '1';
                }
                int tmp_value = 0;
                for(int k = 1;k<str.size();++k) {
                    if(str[k] == '1')
                        tmp_value += pow(2,(str.size()-1-k));
                }
                tmp_value += 1;
                va[i] += to_string(tmp_value);   //
                temp_data.second=(0-tmp_value);
            }
            else if(str[0] == '0') {
                int tmp_value = 0;
                for(int k = 1;k<str.size();++k) {
                    if(str[k] == '1')
                        tmp_value += pow(2,(str.size()-1-k));
                }
                va[i] += to_string(tmp_value);    //whether add \n?
                temp_data.second=tmp_value;
            }
            my_data.push_back(temp_data);
            continue;
        }
        my_map[add]=i;
        if(start_0 == "110") {//第一类，算数运算 rd=rs op rt
            if(data_start_flag==1){
                pair<int,int> temp_data;
                temp_data.first = add; //地址
                string num_str = va[i].substr(0,32);
                if(num_str[0] == '1') {
                    va[i] += '-';
                    for(int j = 1;j<num_str.size();++j) {
                        if(num_str[j] == '1')
                            num_str[j] = '0';
                        else
                            num_str[j] = '1';
                    }
                    int tmp_value = 0;
                    for(int k = 1;k<num_str.size();++k) {
                        if(num_str[k] == '1')
                            tmp_value += pow(2,(num_str.size()-1-k));
                    }
                    tmp_value += 1;
                    va[i] += to_string(tmp_value);
                    temp_data.second=(0-tmp_value);

                }
                else if(num_str[0] == '0') {
                    int tmp_value = 0;
                    for(int k = 1;k<num_str.size();++k) {
                        if(num_str[k] == '1')
                            tmp_value += pow(2,(num_str.size()-1-k));
                    }
                    va[i] += to_string(tmp_value);
                    temp_data.second=(tmp_value);

                }
                my_data.push_back(temp_data);
                continue; //如果是数字就继续
            }
            my_map[add]=i;
            string start_13 = va[i].substr(13,3);//第13个开始截取三位
                if(start_13 == "000")
                    va[i] += "ADD";//ADD 寄存器数字相加
                else if(start_13 =="001")
                    va[i] += "SUB";
                else if(start_13 == "010")
                    va[i] += "MUL";
                else if(start_13 == "011")
                    va[i] += "AND";  //与运算
                else if(start_13 == "100")
                    va[i] += "OR";  //或运算
                else if(start_13 == "101")
                    va[i] += "XOR"; //异或运算
                else if(start_13 == "110")
                    va[i] += "NOR";  //位运算
                va[i] += ' ';
            //get rd
                string start_16 = va[i].substr(16,5);
                int rd_value = 0;
                for(int i = 0;i < start_16.size();++i) {
                    if(start_16[i] == '1')
                        rd_value += pow(2,(start_16.size()-1-i));
                }
                va[i] += 'R' + to_string(rd_value) + ',' + ' ';
            //get rs
                string start_3 = va[i].substr(3,5);
                int rs_value = 0;
                for(int i = 0;i< start_3.size();++i) {
                    if(start_3[i] == '1')
                        rs_value += pow(2,(start_3.size()-1-i));
                }
                va[i] += 'R' + to_string(rs_value) + ',' + ' ';
            //get rt
                string start_8 = va[i].substr(8,5);
                int rt_value = 0;
                for(int i = 0;i<start_8.size();++i) {
                    if(start_8[i] == '1')
                        rt_value += pow(2,(start_8.size()-1-i));
                }
                va[i] += 'R' +to_string(rt_value); //whether add the \n?
        }//end start_0(110)
        else if(start_0 == "111") {//rt=rs op immdediate
            //get opcode
                string start_13 = va[i].substr(13,3);
                if(start_13 == "000")
                    va[i] += "ADDI";
                else if(start_13 =="001")
                    va[i] += "ANDI";
                else if(start_13 == "010")
                    va[i] += "ORI";
                else if(start_13 == "011")
                    va[i] += "XORI";
                va[i] += ' ';
            //get rt
                string start_8 = va[i].substr(8,5);
                int rt_value = 0;
                for(int i = 0;i<start_8.size();++i)
                {
                    if(start_8[i] == '1')
                        rt_value += pow(2,(start_8.size()-1-i));
                }
                va[i] += 'R' +to_string(rt_value) + ',' + ' ';
            //get rs
                string start_3 = va[i].substr(3,5);
                int rs_value = 0;
                for(int i = 0;i< start_3.size();++i)
                {
                    if(start_3[i] == '1')
                        rs_value += pow(2,(start_3.size()-1-i));
                }
                va[i] += 'R' + to_string(rs_value) + ',' + ' ';
            //get immediate_value
                string start_16 = va[i].substr(16,16);
                int im_value = 0;
                for(int i = 0;i < start_16.size();++i)
                {
                    if(start_16[i] == '1')
                        im_value += pow(2,(start_16.size()-1-i));
                }
                va[i] += '#' + to_string(im_value);
        }//end start_0(111)
        else if(start_0 == "000") {// 一些跳转指令
            string start_3 = va[i].substr(3,3);
            if(start_3 == "000") { //J 跳转
                va[i] += "J";
                va[i] += ' ';
                string start_j = va[i].substr(6,26);
                start_j.append("00");
                int j_value = 0;
                for(int i = 0;i < start_j.size();++i) {
                    if(start_j[i] == '1')
                        j_value += pow(2,(start_j.size()-1-i));
                }
                va[i] += '#' + to_string(j_value);
            }// end of J
            else if(start_3 =="010") { //BEQ
                va[i] += "BEQ";
                va[i] += ' ';
                //get rs
                string start_6 = va[i].substr(6,5);
                int rs_value = 0;
                for(int i = 0;i< start_6.size();++i) {
                    if(start_6[i] == '1')
                        rs_value += pow(2,(start_6.size()-1-i));
                }
                va[i] += 'R' + to_string(rs_value) + ',' + ' ';
                //end get rs
                //get rt
                string start_11 = va[i].substr(11,5);
                int rt_value = 0;
                for(int i = 0;i< start_11.size();++i) {
                    if(start_11[i] == '1')
                        rt_value += pow(2,(start_11.size()-1-i));
                }
                va[i] += 'R' + to_string(rt_value) + ',' + ' ';
                //end get rt
                //get offset
                string start_16 = va[i].substr(16,16);
                start_16.append("00");
                int im_value = 0;
                for(int i = 0;i < start_16.size();++i) {
                    if(start_16[i] == '1')
                        im_value += pow(2,(start_16.size()-1-i));
                }
                va[i] += '#' + to_string(im_value);  // whether add \n?
                //end get offset
            }// end of BEQ
            else if(start_3 == "100") {//BGTZ
                va[i] += "BGTZ";
                va[i] += ' ';
                //get rs
                string start_6 = va[i].substr(6,5);
                int rs_value = 0;
                for(int i = 0;i< start_6.size();++i) {
                    if(start_6[i] == '1')
                        rs_value += pow(2,(start_6.size()-1-i));
                }
                va[i] += 'R' + to_string(rs_value) + ',' + ' ';
                //end get rs
                //get offset
                string start_16 = va[i].substr(16,16);
                start_16.append("00");
                int im_value = 0;
                for(int i = 0;i < start_16.size();++i) {
                    if(start_16[i] == '1')
                        im_value += pow(2,(start_16.size()-1-i));
                }
                va[i] += '#' + to_string(im_value);  // whether add \n?
                //end get offset
            }//end of BGTZ
            else if(start_3 == "101") {//BREAK
                va[i] += "BREAK";
                data_start_flag=1;
                data_start=i;
//                break;
            }//end of BREAK
            else if(start_3 == "110") {//存，寄存器存到存储器中
                va[i] += "SW";
                va[i] += ' ';
                //get rt
                string start_11 = va[i].substr(11,5);
                int rt_value = 0;
                for(int i = 0;i< start_11.size();++i) {
                    if(start_11[i] == '1')
                        rt_value += pow(2,(start_11.size()-1-i));
                }
                va[i] += 'R' + to_string(rt_value) + ',' + ' ';
                //end get rt
                //get offset
                string start_16 = va[i].substr(16,16);
                int im_value = 0;
                for(int i = 0;i < start_16.size();++i) {
                    if(start_16[i] == '1')
                        im_value += pow(2,(start_16.size()-1-i));
                }
                va[i] += to_string(im_value);
                //end get offset
                //get base
                string start_6 = va[i].substr(6,5);
                int rs_value = 0;
                for(int i = 0;i< start_6.size();++i) {
                    if(start_6[i] == '1')
                        rs_value += pow(2,(start_6.size()-1-i));
                }
                va[i] += "(R" + to_string(rs_value) + ')';  //whether add \n?
                //end get base
            }
            else if(start_3 == "111") { //LW
                va[i] += "LW";
                va[i] += ' ';
                //get rt
                string start_11 = va[i].substr(11,5);
                int rt_value = 0;
                for(int i = 0;i< start_11.size();++i) {
                    if(start_11[i] == '1')
                        rt_value += pow(2,(start_11.size()-1-i));
                }
                va[i] += 'R' + to_string(rt_value) + ',' + ' ';
                //end get rt
                //get offset
                string start_16 = va[i].substr(16,16);
                int im_value = 0;
                for(int i = 0;i < start_16.size();++i) {
                    if(start_16[i] == '1')
                        im_value += pow(2,(start_16.size()-1-i));
                }
                va[i] += to_string(im_value);
                //end get offset
                //get base
                string start_6 = va[i].substr(6,5);
                int rs_value = 0;
                for(int i = 0;i< start_6.size();++i) {
                    if(start_6[i] == '1')
                        rs_value += pow(2,(start_6.size()-1-i));
                }
                va[i] += "(R" + to_string(rs_value) + ')';  //whether add \n?
                //end get base
            }
        }//end start_0(000)
    }//end for

//    for (map<int, int>::iterator i=my_map.begin(); i!=my_map.end(); i++)
//    {
//        cout<<i->first<<" "<<i->second<<endl;
//    }
//    for(int i=0;i<my_data.size();i++){
//        cout<<my_data[i].first<<" "<<my_data[i].second<<endl;
//    }
//    cout<<"start"<<data_start<<endl;
    //output to file
    ofstream fout("disassembly.txt");
    for(int i = 0; i < va.size(); ++i)
        fout << va[i] << endl;
    fout << flush;
    fout.close();

    simulation(va,data_start);

}
void simulation(vector<string> &va,int start)
{
    int i=start; // data开始地方

    int im_index = (start + 1) * 4 + 128;//
    int cycle_count = 0; // cycle
    bool can_fetch = true; //能不能fetch
    bool can_issue = true; // 能不能issue
    bool can_issue_pre[4] = {false, false, false, false};//每个指令能不能发送
    //这四个临时变量是为了存储操作符的
    string temp_str1;
    string temp_str2;
    string temp_str3;
    string temp_str4;

    //这三个是做运算的时候代表的op,rs,rd的
    int temp_reg1;
    int temp_reg2;
    int temp_reg3;
    int pc = 128; //PC
    int previous_temp = 0;
    bool have_store = false;//SW 在不在 LW 前
    //temp_temp
    int temp_sum_pre_issue_queue = 0; //上一轮issue_queue用的个数 解决Sampel3 的关键
    while(pc > 0) {
        cycle_count++;
        can_issue = true;
        can_fetch = true;
        temp_sum_pre_issue_queue = sum_pre_issue_queue; //记录上回结束pre_issue里面有几个,因为倒着执行。防止满了还能fetch
        //这个是为了存下每次循环开始的时候寄存器的状态，做为后续函数的比较用
        for(int j = 0; j < reg_state2.size(); j++) {
            reg_state2[j] = 1;
        }
        // 每一轮初始状态
        sum_temp2[0] = pre_alu_queue[0];
        sum_temp2[1] = pre_alu_queue[1];
        sum_temp2[2] = pre_mem_queue;
        sum_temp2[3] = post_mem_queue;
        sum_temp2[4] = post_alu_queue;
        sum_temp2[5] = pre_issue_queue[0];
        sum_temp2[6] = pre_issue_queue[1];
        sum_temp2[7] = pre_issue_queue[2];
        sum_temp2[8] = pre_issue_queue[3];
        for(int j = 0; j < sum_temp2.size(); j++) {// 设置rs寄存器的状态为写，避免WAW，WAR
            if(sum_temp2[j] != "") {
                temp_str3 = sum_temp2[j].substr(37);
                cout<<sum_temp2[j]<<endl;
                temp_str4 = temp_str3.substr(0,4);
                if(temp_str4 == "ADDI" || temp_str4 == "ANDI" || temp_str4 == "XORI" || temp_str4 == "ORI ") {
                    //get rt
                    string start_8 = sum_temp2[j].substr(8,5);
                    int rt_value = 0;
                    for(int i = 0;i<start_8.size();++i) {
                        if(start_8[i] == '1')
                            rt_value += pow(2,(start_8.size()-1-i));
                    }
                    temp_reg1 = rt_value;
                    reg_state2[temp_reg1] = 0;
                }
                if(temp_str4 == "ADD " || temp_str4 == "SUB " || temp_str4 == "MUL " || temp_str4 == "AND " || temp_str4 == "XOR " || temp_str4 == "NOR " || temp_str4 == "OR R"   ) {
                    //get rd
                    string start_16 = sum_temp2[j].substr(16,5);
                    int rd_value = 0;
                    for(int i = 0;i < start_16.size();++i) {
                        if(start_16[i] == '1')
                            rd_value += pow(2,(start_16.size()-1-i));
                    }
                    temp_reg1 = rd_value;
                    reg_state2[temp_reg1] = 0;
                }//END OF ADD SUB MUL ..
                if(temp_str4 == "LW R") {
                    //get rt
                    string start_11 = sum_temp2[j].substr(11,5);
                    int rt_value = 0;
                    for(int i = 0;i< start_11.size();++i) {
                        if(start_11[i] == '1')
                            rt_value += pow(2,(start_11.size()-1-i));
                    }
                    temp_reg1 = rt_value;
                    reg_state2[temp_reg1] = 0; //2 记录每一轮结束寄存器的状态,倒着执行要记录状态做为后期判断条件。
                }//end of LW
            }//end of != ""
        }//end of for
        /*
         * IF 部分 这一部分主要是处理IF unit 的两个寄存器，BEQ和 BGTZ 在UNIT[0]里面
         * BREAK 和 J 在UNIT[1]中
         */
        if(unit[0]!=""||unit[1]!="") {
            if(unit[0] != "") {
                //get opcode
                string start_3 = unit[0].substr(3,3);//IF  unit第一个槽不是空
                if(start_3 =="010") {//BEQ
                    //get rs
                    string start_6 = unit[0].substr(6,5);
                    int rs_value = 0;
                    for(int i = 0;i< start_6.size();++i) {
                        if(start_6[i] == '1')
                            rs_value += pow(2,(start_6.size()-1-i));
                    }
                    //get rt
                    string start_11 = unit[0].substr(11,5);
                    int rt_value = 0;
                    for(int i = 0;i< start_11.size();++i) {
                        if(start_11[i] == '1')
                            rt_value += pow(2,(start_11.size()-1-i));
                    }
                    if(reg_state2[rs_value] != 0 && reg_state2[rt_value] != 0) {
                        unit[1] = unit[0];
                        unit[0] = "";
                    } //如果不存在冒险，则走到下一步
                }// end of BEQ
                if(start_3 == "100") {//BGTZ
                    string start_6 = unit[0].substr(6,5);
                    int rs_value = 0;
                    for(int i = 0;i< start_6.size();++i) {
                        if(start_6[i] == '1')
                            rs_value += pow(2,(start_6.size()-1-i));
                    }
                    if(reg_state2[rs_value] != 0) {
                        unit[1] = unit[0];
                        unit[0] = "";
                    }
                }//end of BGTZ
            }//end of unit[0] != ""
            else {//unit[1] != "" 第二个不是空
                //get opcode
                string start_3 = unit[1].substr(3,3);
                if(start_3 =="010") { //BEQ
                    string start_6 = unit[1].substr(6,5);
                    int rs_value = 0;
                    for(int i = 0;i< start_6.size();++i) {
                        if(start_6[i] == '1')
                            rs_value += pow(2,(start_6.size()-1-i));
                    }
                    //get rt
                    string start_11 = unit[1].substr(11,5);
                    int rt_value = 0;
                    for(int i = 0;i< start_11.size();++i) {
                        if(start_11[i] == '1')
                            rt_value += pow(2,(start_11.size()-1-i));
                    }
                    string start_16 = unit[1].substr(16,16);
                    start_16.append("00");
                    int im_value = 0;
                    for(int i = 0;i < start_16.size();++i) {
                        if(start_16[i] == '1')
                            im_value += pow(2,(start_16.size()-1-i));
                    }
                    if(reg_state2[rs_value] != 0 && reg_state2[rt_value] != 0) {
                        if(reg[rs_value] == reg[rt_value]) {
                            pc = pc + im_value;
                        }
                        unit[1] = "";
                    }
                }// end of BEQ 这个时候就是该执行了。
                if(start_3 == "100") {// BGTZ
                    string start_6 = unit[1].substr(6,5);
                    int rs_value = 0;
                    for(int i = 0;i< start_6.size();++i) {
                        if(start_6[i] == '1')
                            rs_value += pow(2,(start_6.size()-1-i));
                    }
                    string start_16 = unit[1].substr(16,16);
                    start_16.append("00");
                    int im_value = 0;
                    for(int i = 0;i < start_16.size();++i) {
                        if(start_16[i] == '1')
                            im_value += pow(2,(start_16.size()-1-i));
                    }
                    if(reg_state2[rs_value] != 0) {
                        if(reg[rs_value] > 0) {
                            pc = pc + im_value;
                        }
                        unit[1] = "";
                    }
                }//end of BGTZ
                if(start_3 == "000") { //J
                    string start_j = unit[1].substr(6,26);
                    start_j.append("00");
                    int j_value = 0;
                    for(int i = 0;i < start_j.size();++i) {
                        if(start_j[i] == '1')
                            j_value += pow(2,(start_j.size()-1-i));
                    }
                    pc = j_value;
                    unit[1] = "";
                }// end of J
                if(start_3 == "101") {//BREAK
                    output_simulation(cycle_count);
                    break;
                }//end of BREAK
            } //end of unit[1] != ""
        }
        //end of IF unit start
        //start_WB
        /*
         * 这一部分就两块看post_ALU里面有没有东西，有东西就进行计算
         * 看看post_mem里面有没有LW。有就进行相应的操作let`s go
         *
         */
        if(post_alu_queue != "") {//得看看post_alu_que里面空不空,不空就要执行
            string start_0 = post_alu_queue.substr(0,3);
            if(start_0 == "110") {//ADD 之类的东西
                //get rd
                string start_16 = post_alu_queue.substr(16,5);
                int rd_value = 0;
                for(int i = 0;i < start_16.size();++i) {
                    if(start_16[i] == '1')
                        rd_value += pow(2,(start_16.size()-1-i));
                }
                string start_3 = post_alu_queue.substr(3,5);
                int rs_value = 0;
                for(int i = 0;i< start_3.size();++i) {
                    if(start_3[i] == '1')
                        rs_value += pow(2,(start_3.size()-1-i));
                }
                //get rt
                string start_8 = post_alu_queue.substr(8,5);
                int rt_value = 0;
                for(int i = 0;i<start_8.size();++i) {
                    if(start_8[i] == '1')
                        rt_value += pow(2,(start_8.size()-1-i));
                }
                string start_13 = post_alu_queue.substr(13,3);
                if(start_13 == "000") {//ADD
                    reg[rd_value] = reg[rs_value] + reg[rt_value];
                }
                else if(start_13 =="001") {//SUB
                    reg[rd_value] = reg[rs_value] - reg[rt_value];
                }
                else if(start_13 == "010") {//MUL
                    reg[rd_value] = reg[rs_value] * reg[rt_value];
                }
                else if(start_13 == "011") {//AND
                    reg[rd_value] = reg[rs_value] & reg[rt_value];
                }
                else if(start_13 == "100") {//OR
                    reg[rd_value] = reg[rs_value] | reg[rt_value];
                }
                else if(start_13 == "101") {//xOR
                    reg[rd_value] = reg[rs_value] ^ reg[rt_value];
                }
                else if(start_13 == "110") {//NOR
                    reg[rd_value] = ~(reg[rs_value] | reg[rt_value]);
                }
            }//end start_0(110)
            if(start_0 == "111") {//ADDI ,ANDI,ORI,XORI之类的
                //get rt
                string start_8 = post_alu_queue.substr(8,5);
                int rt_value = 0;
                for(int i = 0;i<start_8.size();i++) {
                    if(start_8[i] == '1')
                        rt_value += pow(2,(start_8.size()-1-i));
                }
                //get rs
                string start_3 = post_alu_queue.substr(3,5);
                int rs_value = 0;
                for(int i = 0;i< start_3.size();++i) {
                    if(start_3[i] == '1')
                        rs_value += pow(2,(start_3.size()-1-i));
                }
                //get immediate_value
                string start_16 = post_alu_queue.substr(16,16);
                int im_value = 0;
                for(int i = 0;i < start_16.size();++i) {
                    if(start_16[i] == '1')
                        im_value += pow(2,(start_16.size()-1-i));
                }
                //get opcode
                string start_13 = post_alu_queue.substr(13,3);
                if(start_13 == "000") {//ADDI
                    reg[rt_value] = reg[rs_value] + im_value;
                }
                else if(start_13 =="001") {//ANDI
                    reg[rt_value] = reg[rs_value] & im_value;
                }
                else if(start_13 == "010") {//ORI
                    reg[rt_value] = reg[rs_value] | im_value;
                }
                else if(start_13 == "011") {//XORI
                    reg[rt_value] = reg[rs_value] ^ im_value;
                }
                //end of get opcode
            }//end start_0(111)
            post_alu_queue = "";
        }
        if(post_mem_queue != "") {//这个一定LW
            //get rt
            string start_11 = post_mem_queue.substr(11,5);
            int rt_value = 0;
            for(int i = 0;i< start_11.size();++i) {
                if(start_11[i] == '1')
                    rt_value += pow(2,(start_11.size()-1-i));
            }
            string start_16 = post_mem_queue.substr(16,16);
            int im_value = 0;
            for(int i = 0;i < start_16.size();++i) {
                if(start_16[i] == '1')
                    im_value += pow(2,(start_16.size()-1-i));
            }

            string start_6 = post_mem_queue.substr(6,5);
            int rs_value = 0;
            for(int i = 0;i< start_6.size();++i) {
                if(start_6[i] == '1')
                    rs_value += pow(2,(start_6.size()-1-i));
            }

            int store_index = ((im_value + reg[rs_value]) - im_index) / 4;//im_index 是数据开始的地方通过偏移可以求出来
            reg[rt_value] = my_data[store_index].second;//通过之前的索引
            post_mem_queue = "";
        }

        //end of WB unit part

        /*
         * mem_start  这一部分只需要看pre_men 有没有东西。如果都的话分情况。
         * 如果是LW就进去post-mem中
         * 如果是sw就运行 let`s go
         */
        if(pre_mem_queue!=""){
            string temp_str4= pre_mem_queue.substr(37);
            temp_str3 = temp_str4.substr(0,2); //这是看看是SW还是LW
            if(temp_str3 == "LW") {
                post_mem_queue = pre_mem_queue;//如果是LW 就直接进入post_men_queue.这里不需要进行判断
                pre_mem_queue = "";
            }
            if(temp_str3 == "SW") { //SW的话要在这里执行完，SW不进post_Man
                //get rt
                string start_11 = pre_mem_queue.substr(11,5);
                int rt_value = 0;
                for(int i = 0;i< start_11.size();++i) {
                    if(start_11[i] == '1')
                        rt_value += pow(2,(start_11.size()-1-i));
                }
                //get offset
                string start_16 = pre_mem_queue.substr(16,16);
                int im_value = 0;
                for(int i = 0;i < start_16.size();++i) {
                    if(start_16[i] == '1')
                        im_value += pow(2,(start_16.size()-1-i));
                }
                //get base
                string start_6 = pre_mem_queue.substr(6,5);
                int rs_value = 0;
                for(int i = 0;i< start_6.size();++i) {
                    if(start_6[i] == '1')
                        rs_value += pow(2,(start_6.size()-1-i));
                }
                int store_index = ((im_value + reg[rs_value]) - im_index) / 4;
                my_data[store_index].second = reg[rt_value];
                pre_mem_queue = "";
            }
        }
        //mem 完了
        /*
        * ALU 这一部分主要就是判断pre_alu 里面到底是LW，SW还是 别的运算符
        * 如果是LW，SW 就放在pre_mem中
        * 如果是其他的运算符就放在post_alu中 let`s go
        */
        //ALU unit part
        previous_temp = sum_pre_alu_queue;//这里备个份
        if(sum_pre_alu_queue > 0) {// alu_queue不为空
            temp_str4 = pre_alu_queue[0].substr(37);// 看看pre_alu里面的存的是什么
            temp_str3 = temp_str4.substr(0,2);
            if(temp_str3 == "LW" || temp_str3 == "SW") {
                pre_mem_queue = pre_alu_queue[0];//如果是LW SW 就放进pre_men_queue
            }
            else {// 去过是其他的操作符 就进post_alu_queue
                post_alu_queue =pre_alu_queue[0];
            }
            if(sum_pre_alu_queue == 2) {//如果是2个 就往前面挪一个，
                pre_alu_queue[0] = pre_alu_queue[1];
                pre_alu_queue[1] = "";
            }
            else {//post_alu_queue 只有一个的话就直接把这个挪出去
                pre_alu_queue[0] = "";
            }
            sum_pre_alu_queue--; // 更新数量
        }
        //end of ALU unit part
        /*
         * 这一部分是issue，核心难点
         * 1. 防止结构冒险pre_alu有空位置
         * 2. 就是说在没有完成的指令集中没有WAW冒险
         * 3. 如果发送两条指令，这两条只能不能有WAW，WAR冒险
         * 4. 在没有发射的队列中也不能有WAR
         * 5. 对于mem这种指令要等一切准备就绪
         * 6. LW要在SW之后
         * 7. SW要按照顺序来
         *`总之就是一个一个去发送，不能有结构冒险和数据冒险。
         * 这里的解决方法就是issue的时候看上一次alu，mem里面寄存器的状态。
         * 并且更新本身的状态防止这条指令之前出现冒险
         * 设置一个标记把SW放在LW之前运行。let`s go
         */
        have_store = false;
        if(pre_alu_queue[0] != "") {//pre_alu_queue是满的
            can_issue = false;
        }
        if(sum_pre_issue_queue == 0) {//自己是空的
            can_issue = false;
        }
        if(can_issue) {
            for(int j = 0; j < sum_pre_issue_queue; j++) {//这个FOR循环为了看哪一个能ISSUE
                temp_str3 = pre_issue_queue[j].substr(37);
                temp_str4 = temp_str3.substr(0,4);
                if(temp_str4 == "ADDI" || temp_str4 == "ANDI" || temp_str4 == "XORI" || temp_str4 == "ORI ") {
                    //get rt
                    string start_8 = pre_issue_queue[j].substr(8,5);
                    int rt_value = 0;
                    for(int i = 0;i<start_8.size();++i) {
                        if(start_8[i] == '1')
                            rt_value += pow(2,(start_8.size()-1-i));
                    }//如果是个反
                    string start_3 = pre_issue_queue[j].substr(3,5);
                    int rs_value = 0;
                    for(int i = 0;i< start_3.size();++i) {
                        if(start_3[i] == '1')
                            rs_value += pow(2,(start_3.size()-1-i));
                    }
                    temp_reg1 = rt_value; //改变寄存器状态
                    temp_reg2 = rs_value;
                    if( reg_state[temp_reg1] == 0 || reg_state[temp_reg2] == 0){//这个东西在上一轮周期之前更新过遍历的是ALU，mem里面的指令{// rt rs 寄存器状态在被写
                        can_issue_pre[j] = false; //不能issue
                        if(reg_state[temp_reg1] != 0) {//rt改成自己要写  这是为了避免issue队列里面产生WAW，WAR
                            reg_state[temp_reg1] = 0;
                        }
                        if(reg_state[temp_reg2] != 0) {
                            reg_state[temp_reg2] = 2;  //read
                        }
                    }
                    else if(reg_state[temp_reg1] == 2) {// 这东西正在被读 RAW
                        can_issue_pre[j] = false;//也不能发射
                        reg_state[temp_reg1] = 0;//
                        if(reg_state[temp_reg2] != 0) {
                            reg_state[temp_reg2] = 2;  //read
                        }
                    }
                    else {
                        can_issue_pre[j] = true;
                        reg_state[temp_reg1] = 0;
                        if(reg_state[temp_reg2] != 0) {
                            reg_state[temp_reg2] = 2;  //read
                        }
                    }
                }//END OF ADDI ANDI..
                if(temp_str4 == "ADD " || temp_str4 == "SUB " || temp_str4 == "MUL " || temp_str4 == "AND " || temp_str4 == "XOR " || temp_str4 == "NOR " || temp_str4 == "OR R") {
                    //get rd
                    string start_16 = pre_issue_queue[j].substr(16,5);
                    int rd_value = 0;
                    for(int i = 0;i < start_16.size();++i) {
                        if(start_16[i] == '1')
                            rd_value += pow(2,(start_16.size()-1-i));
                    }
                    //get rs
                    string start_3 = pre_issue_queue[j].substr(3,5);
                    int rs_value = 0;
                    for(int i = 0;i< start_3.size();++i) {
                        if(start_3[i] == '1')
                            rs_value += pow(2,(start_3.size()-1-i));
                    }
                    string start_8 = pre_issue_queue[j].substr(8,5);
                    int rt_value = 0;
                    for(int i = 0;i<start_8.size();++i) {
                        if(start_8[i] == '1')
                            rt_value += pow(2,(start_8.size()-1-i));
                    }
                    temp_reg1 = rd_value;
                    temp_reg2 = rs_value;
                    temp_reg3 = rt_value;

                    if( reg_state[temp_reg1] == 0 || reg_state[temp_reg2] == 0 || reg_state[temp_reg3] == 0) {
                        can_issue_pre[j] = false;
                        if(reg_state[temp_reg1] != 0) {
                            reg_state[temp_reg1] = 0;
                        }
                        if(reg_state[temp_reg2] != 0) {
                            reg_state[temp_reg2] = 2;
                        }
                        if(reg_state[temp_reg3] != 0) {
                            reg_state[temp_reg3] = 2;
                        }
                    }
                    else if(reg_state[temp_reg1] == 2) {
                        can_issue_pre[j] = false;
                        reg_state[temp_reg1] = 0;
                        if(reg_state[temp_reg2] != 0) {
                            reg_state[temp_reg2] = 2;  //read
                        }
                        if(reg_state[temp_reg3] != 0) {
                            reg_state[temp_reg3] = 2;  //read
                        }
                    }
                    else {
                        can_issue_pre[j] = true;
                        reg_state[temp_reg1] = 0;
                        if(reg_state[temp_reg2] != 0) {
                            reg_state[temp_reg2] = 2;  //read
                        }
                        if(reg_state[temp_reg3] != 0)
                        {
                            reg_state[temp_reg3] = 2;  //read
                        }
                    }
                }//end 第二类
                if(temp_str4 == "SW R") {
                    //get rt
                    string start_11 = pre_issue_queue[j].substr(11,5);
                    int rt_value = 0;
                    for(int i = 0;i< start_11.size();++i) {
                        if(start_11[i] == '1')
                            rt_value += pow(2,(start_11.size()-1-i));
                    }
                    string start_6 = pre_issue_queue[j].substr(6,5);
                    int rs_value = 0;
                    for(int i = 0;i< start_6.size();++i) {
                        if(start_6[i] == '1')
                            rs_value += pow(2,(start_6.size()-1-i));
                    }
                    temp_reg1 = rt_value;
                    temp_reg2 = rs_value;
                    if( reg_state[temp_reg1] == 0 || reg_state[temp_reg2] == 0) {
                        can_issue_pre[j] = false;
                        if(reg_state[temp_reg1] != 0) {
                            reg_state[temp_reg1] = 2;
                        }
                        if(reg_state[temp_reg2] != 0) {
                            reg_state[temp_reg2] = 2;
                        }
                    }
                    else {
                        can_issue_pre[j] = true;
                        if(reg_state[temp_reg1] != 0) {
                            reg_state[temp_reg1] = 2;
                        }
                        if(reg_state[temp_reg2] != 0) {
                            reg_state[temp_reg2] = 2;
                        }
                    }
                    have_store = true;
                }//END OF SW
                if(temp_str4 == "LW R") {
                    //get rt
                    string start_11 = pre_issue_queue[j].substr(11,5);
                    int rt_value = 0;
                    for(int i = 0;i< start_11.size();++i) {
                        if(start_11[i] == '1')
                            rt_value += pow(2,(start_11.size()-1-i));
                    }
                    string start_6 = pre_issue_queue[j].substr(6,5);
                    int rs_value = 0;
                    for(int i = 0;i< start_6.size();++i) {
                        if(start_6[i] == '1')
                            rs_value += pow(2,(start_6.size()-1-i));
                    }
                    temp_reg1 = rt_value;
                    temp_reg2 = rs_value;
                    if( reg_state[temp_reg1] == 0 || reg_state[temp_reg2] == 0) {
                        can_issue_pre[j] = false;
                        if(reg_state[temp_reg1] != 0) {
                            reg_state[temp_reg1] = 0;
                        }
                        if(reg_state[temp_reg2] != 0) {
                            reg_state[temp_reg2] = 2;
                        }
                    }
                    else if(reg_state[temp_reg1] == 2) {
                        can_issue_pre[j] = false;
                        reg_state[temp_reg1] = 0;
                        if(reg_state[temp_reg2] != 0) {
                            reg_state[temp_reg2] = 2;
                        }
                    }
                    else {
                        can_issue_pre[j] = true;
                        reg_state[temp_reg1] = 0;
                        if(reg_state[temp_reg2] != 0) {
                            reg_state[temp_reg2] = 2;  //read
                        }
                    }
                    if(have_store) {//sw 在前面 就不能执行LW
                        can_issue_pre[j] = false;
                    }
                }//end of LW
            }
            /*
            * 更新pre_issue_queue
            */
            int can_issue_count = 0;
            if(previous_temp == 1) {// 就是ALU que里面的个数
                can_issue_count = 1;
            }
            if(previous_temp == 2) {
                can_issue_count = 0;
            }
            if(previous_temp == 0) {
                can_issue_count = 2;
            }
            for(int j = 0; j < sum_pre_issue_queue && can_issue_count > 0; j++) {
                if(can_issue_pre[j]) {
                    pre_alu_queue[sum_pre_alu_queue] = pre_issue_queue[j];
                    sum_pre_alu_queue++;
                    can_issue_count--;
                    for(i = j; i < sum_pre_issue_queue - 1; i++) {
                        pre_issue_queue[i] = pre_issue_queue[i+1];
                        can_issue_pre[i] = can_issue_pre[i+1];
                    }
                    can_issue_pre[sum_pre_issue_queue-1] = false;
                    pre_issue_queue[sum_pre_issue_queue-1] = "";
                    sum_pre_issue_queue--;
                    j--;
                }
            }
            cout<<have_store<<endl;
        }
        /*
        *  fetch 部分 这一部分主要为了防止分支指令的存在就不能fetch
        *  分支指令就有BEQ， BGTZ ,进入If_unit[0],J ,BREAk 进入If_unit[1].let`s go
        */
        can_fetch = true;
        if(unit[0] != "" || unit[1] != "") {// IF UNIT这里两个任何一个地方为空都不行
            can_fetch = false;
        } //分支等待。

        if(temp_sum_pre_issue_queue == 4){
            can_fetch = false;
        }// 上一轮的ISSUE队列如果是满的也不行


        if(can_fetch) {
            temp_str1 = va[my_map[pc]].substr(37);
            if(temp_str1.substr(0,3) == "BEQ" || temp_str1.substr(0,4) == "BGTZ") {
                unit[0] = va[my_map[pc]];
                can_fetch = false;
            }//如果有分支指令下一条就不能fech了
            else if(temp_str1.substr(0,1) == "J") {
                unit[1] = va[my_map[pc]];
                can_fetch = false;
            }
            else if(temp_str1.substr(0,5) == "BREAK") {
                unit[1] = va[my_map[pc]];
                can_fetch = false;
                pc = -100000;
            }
            else {//其他的操作符
                pre_issue_queue[sum_pre_issue_queue] = va[my_map[pc]];
                sum_pre_issue_queue++;
            }
            pc += 4;
        }

        if(unit[0] != "" || unit[1] != "") {
            can_fetch = false;
        }

        if(temp_sum_pre_issue_queue == 3)
            can_fetch = false;

        if(can_fetch) { //看第二次能不能fetch
            temp_str2 = va[my_map[pc]].substr(37);
            if(temp_str2.substr(0,3) == "BEQ" || temp_str2.substr(0,4) == "BGTZ") {
                unit[0] = va[my_map[pc]];
                can_fetch = false;
            }
            else if(temp_str2.substr(0,1) == "J") {
                unit[1] = va[my_map[pc]];
                can_fetch = false;
            }
            else if(temp_str2.substr(0,5) == "BREAK") {
                unit[1] = va[my_map[pc]];
                can_fetch = false; //
                pc = -1000;
            }
            else {
                pre_issue_queue[sum_pre_issue_queue] = va[my_map[pc]];
                sum_pre_issue_queue++;
            }
            pc += 4;
        }


        for(int j = 0; j < reg_state.size(); j++) {
            reg_state[j] = 1;
        }

        sum_temp[0] = pre_alu_queue[0];
        sum_temp[1] = pre_alu_queue[1];
        sum_temp[2] = pre_mem_queue;
        sum_temp[3] = post_mem_queue;
        sum_temp[4] = post_alu_queue;


        //更新ALU,mem,中指令对应寄存器的状态。因为下一次issue的时候要用得到。
        for(int j = 0; j < sum_temp.size(); j++) {
            if(sum_temp[j] != "") {
                temp_str3 = sum_temp[j].substr(37);
                temp_str4 = temp_str3.substr(0,4);
                if(temp_str4 == "ADDI" || temp_str4 == "ANDI" || temp_str4 == "XORI" || temp_str4 == "ORI ") {
                    //get rt
                    string start_8 = sum_temp[j].substr(8,5);
                    int rt_value = 0;
                    for(int i = 0;i<start_8.size();++i) {
                        if(start_8[i] == '1')
                            rt_value += pow(2,(start_8.size()-1-i));
                    }
                    temp_reg1 = rt_value;
                    reg_state[temp_reg1] = 0;
                }//END OF ADDI ANDI..

                if(temp_str4 == "ADD " || temp_str4 == "SUB " || temp_str4 == "MUL " || temp_str4 == "AND " || temp_str4 == "XOR " || temp_str4 == "NOR " ||temp_str4 == "OR R") {
                    //get rd
                    string start_16 = sum_temp[j].substr(16,5);
                    int rd_value = 0;
                    for(int i = 0;i < start_16.size();++i) {
                        if(start_16[i] == '1')
                            rd_value += pow(2,(start_16.size()-1-i));
                    }
                    temp_reg1 = rd_value;
                    reg_state[temp_reg1] = 0;
                }//END OF ADD SUB MUL ..
                if(temp_str4 == "LW R") {
                    //get rt
                    string start_11 = sum_temp[j].substr(11,5);
                    int rt_value = 0;
                    for(int i = 0;i< start_11.size();++i)
                    {
                        if(start_11[i] == '1')
                            rt_value += pow(2,(start_11.size()-1-i));
                    }
                    temp_reg1 = rt_value;
                    reg_state[temp_reg1] = 0;
                }//end of LW
            }//end of != ""
        }//end of for
        output_simulation(cycle_count);
    }
}//end of simulation
void output_simulation(int cycle)
{
    int i;
    ofstream my_out("simulation.txt",ios::app);
    my_out << "--------------------" << endl;
    my_out << "Cycle:" << cycle <<endl;
    my_out << endl;
    my_out << "IF Unit:" << endl;
    my_out << "\t" << "Waiting Instruction:";
    if(unit[0] != "") {
        my_out << "[";
        my_out << unit[0].substr(37);
        my_out << "]";
    }
    my_out << endl;
    my_out << "\t" << "Executed Instruction:";
    if(unit[1] != "") {
        my_out << "[";
        my_out << unit[1].substr(37);
        my_out << "]";
    }
    my_out << endl;
    my_out << "Pre-Issue Queue:" << endl;
    my_out << "\t" << "Entry 0:";
    if(pre_issue_queue[0] != "") {
        my_out << "[";
        my_out << pre_issue_queue[0].substr(37);
        my_out << "]";
    }
    my_out << endl;
    my_out << "\t" << "Entry 1:";
    if(pre_issue_queue[1] != "") {
        my_out << "[";
        my_out << pre_issue_queue[1].substr(37);
        my_out << "]";
    }
    my_out << endl;
    my_out << "\t" << "Entry 2:";
    if(pre_issue_queue[2] != "") {
        my_out << "[";
        my_out << pre_issue_queue[2].substr(37);
        my_out << "]";
    }
    my_out << endl;
    my_out << "\t" << "Entry 3:";
    if(pre_issue_queue[3] != "") {
        my_out << "[";
        my_out << pre_issue_queue[3].substr(37);
        my_out << "]";
    }
    my_out << endl;
    my_out << "Pre-ALU Queue:" << endl;
    my_out << "\t" << "Entry 0:";
    if(pre_alu_queue[0] != "") {
        my_out << "[";
        my_out << pre_alu_queue[0].substr(37);
        my_out << "]";
    }
    my_out << endl;
    my_out << "\t" << "Entry 1:";
    if(pre_alu_queue[1] != "") {
        my_out << "[";
        my_out << pre_alu_queue[1].substr(37);
        my_out << "]";
    }
    my_out << endl;
    my_out << "Pre-MEM Queue:";
    if(pre_mem_queue != "")
    {
        my_out << "[";
        my_out << pre_mem_queue.substr(37);
        my_out << "]";
    }
    my_out << endl;
    my_out << "Post-MEM Queue:";
    if(post_mem_queue != "") {
        my_out << "[";
        my_out << post_mem_queue.substr(37);
        my_out << "]";
    }
    my_out << endl;
    my_out << "Post-ALU Queue:";
    if(post_alu_queue != "") {
        my_out << "[";
        my_out << post_alu_queue.substr(37);
        my_out << "]";
    }
    my_out << endl;
    my_out << endl;
    my_out << "Registers" << endl;
    for(i=0;i<reg.size();++i) {
        if(i % 8 == 0) {
            my_out << 'R';
            my_out << setw(2) << setfill('0') << i;
            my_out << ':';
        }
        my_out << '\t' << reg[i];
        if(i % 8 == 7) {
            my_out << endl;
        }
    }
    my_out << endl;
    my_out << "Data" << endl;
    for(i=0;i<my_data.size();++i) {
        if(i % 8 == 0) {
            my_out << my_data[i].first << ':';
        }
        my_out << '\t' << my_data[i].second;
        if(i % 8 == 7) {
            my_out << endl;
        }
    }
    my_out << endl;
    my_out << flush;
    my_out.close();
}

