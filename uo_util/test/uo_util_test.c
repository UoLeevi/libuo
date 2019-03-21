#include "uo_util.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

const char *lorem = "elementum nibh tellus molestie nunc non blandit massa enim nec dui nunc mattis enim ut tellus elementum sagittis vitae et leo duis ut diam quam nulla porttitor massa id neque aliquam vestibulum morbi blandit cursus risus at ultrices mi tempus imperdiet nulla malesuada pellentesque elit eget gravida cum sociis natoque penatibus et magnis dis parturient montes nascetur ridiculus mus mauris vitae ultricies leo integer malesuada nunc vel risus commodo viverra maecenas accumsan lacus vel facilisis volutpat est velit egestas dui id ornare arcu odio ut sem nulla pharetra diam sit amet nisl suscipit adipiscing bibendum est ultricies integer quis auctor elit sed vulputate mi sit amet mauris commodo quis imperdiet massa tincidunt nunc pulvinar sapien et ligula ullamcorper malesuada proin libero nunc consequat interdum varius sit amet mattis vulputate enim nulla aliquet porttitor lacus luctus accumsan tortor posuere ac ut consequat semper viverra nam libero justo laoreet sit amet cursus sit amet dictum sit amet justo donec enim diam vulputate ut pharetra sit amet aliquam id diam maecenas ultricies mi eget mauris pharetra et ultrices neque ornare aenean euismod elementum nisi quis eleifend quam adipiscing vitae proin sagittis nisl rhoncus mattis rhoncus urna neque viverra justo nec ultrices dui sapien eget mi proin sed libero enim sed faucibus turpis in eu mi bibendum neque egestas congue quisque egestas diam in arcu cursus euismod quis viverra nibh cras pulvinar mattis nunc sed blandit libero volutpat sed cras ornare arcu dui vivamus arcu felis bibendum ut tristique et egestas quis ipsum suspendisse ultrices gravida dictum fusce ut placerat orci nulla pellentesque dignissim enim sit amet venenatis urna cursus eget nunc scelerisque viverra mauris in aliquam sem fringilla ut morbi tincidunt augue interdum velit euismod in pellentesque massa placerat duis ultricies lacus sed turpis tincidunt id aliquet risus feugiat in ante metus dictum at tempor commodo ullamcorper a lacus vestibulum sed arcu non odio euismod lacinia at quis risus sed vulputate odio ut enim blandit volutpat maecenas volutpat blandit aliquam etiam erat velit scelerisque in dictum non consectetur a erat nam at lectus urna duis convallis convallis tellus id interdum velit laoreet id donec ultrices tincidunt arcu non sodales neque sodales ut etiam sit amet nisl purus in mollis nunc sed id semper risus in hendrerit gravida rutrum quisque non tellus orci ac auctor augue mauris augue neque gravida in fermentum et sollicitudin ac orci phasellus egestas tellus rutrum tellus pellentesque eu tincidunt tortor aliquam nulla facilisi cras fermentum odio eu feugiat pretium nibh ipsum consequat nisl vel pretium lectus quam id leo in vitae turpis massa sed elementum tempus egestas sed sed risus pretium quam vulputate dignissim suspendisse in est ante in nibh mauris cursus mattis molestie a iaculis at erat pellentesque adipiscing commodo elit at imperdiet dui accumsan sit amet nulla facilisi morbi tempus iaculis urna id volutpat lacus laoreet non curabitur gravida arcu ac tortor dignissim elementum nibh tellus molestie nunc non blandit massa enim nec dui nunc mattis enim ut tellus elementum sagittis vitae et leo duis ut diam quam nulla porttitor massa id neque aliquam vestibulum morbi blandit cursus risus at ultrices mi tempus imperdiet nulla malesuada pellentesque elit eget gravida cum sociis natoque penatibus et magnis dis parturient montes nascetur ridiculus mus mauris vitae ultricies leo integer malesuada nunc vel risus commodo viverra maecenas accumsan lacus vel facilisis volutpat est velit egestas dui id ornare arcu odio ut sem nulla pharetra diam sit amet nisl suscipit adipiscing bibendum est ultricies integer quis auctor elit sed vulputate mi sit amet mauris commodo quis imperdiet massa tincidunt nunc pulvinar sapien et ligula ullamcorper malesuada proin libero nunc consequat interdum varius sit amet mattis vulputate enim nulla aliquet porttitor lacus luctus accumsan tortor posuere ac ut consequat semper viverra nam libero justo laoreet sit amet cursus sit amet dictum sit amet justo donec enim diam vulputate ut pharetra sit amet aliquam id diam maecenas ultricies mi eget mauris pharetra et ultrices neque ornare aenean euismod elementum nisi quis eleifend quam adipiscing vitae proin sagittis nisl rhoncus mattis rhoncus urna neque viverra justo nec ultrices dui sapien eget mi proin sed libero enim sed faucibus turpis in eu mi bibendum neque egestas congue quisque egestas diam in arcu cursus euismod quis viverra nibh cras pulvinar mattis nunc sed blandit libero volutpat sed cras ornare arcu dui vivamus arcu felis bibendum ut tristique et egestas quis ipsum suspendisse ultrices gravida dictum fusce ut placerat orci nulla pellentesque dignissim enim sit amet venenatis urna cursus eget nunc scelerisque viverra mauris in aliquam sem fringilla ut morbi tincidunt augue interdum velit euismod in pellentesque massa placerat duis ultricies lacus sed turpis tincidunt id aliquet risus feugiat in ante metus dictum at tempor commodo ullamcorper a lacus vestibulum sed arcu non odio euismod lacinia at quis risus sed vulputate odio ut enim blandit volutpat maecenas volutpat blandit aliquam etiam erat velit scelerisque in dictum non consectetur a erat nam at lectus urna duis convallis convallis tellus id interdum velit laoreet id donec ultrices tincidunt arcu non sodales neque sodales ut etiam sit amet nisl purus in mollis nunc sed id semper risus in hendrerit gravida rutrum quisque non tellus orci ac auctor augue mauris augue neque gravida in fermentum et sollicitudin ac orci phasellus egestas tellus rutrum tellus pellentesque eu tincidunt tortor aliquam nulla facilisi cras fermentum odio eu feugiat pretium nibh ipsum consequat nisl vel pretium lectus quam id leo in vitae turpis massa sed elementum tempus egestas sed sed risus pretium quam vulputate dignissim suspendisse in est ante in nibh mauris cursus mattis molestie a iaculis at erat pellentesque adipiscing commodo elit at imperdiet dui accumsan sit amet nulla facilisi morbi tempus iaculis urna id volutpat lacus laoreet non curabitur gravida arcu ac tortor dignissim convallis aenean et tortor at risus viverra adipiscing at in tellus integer feugiat scelerisque varius morbi enim nunc faucibus a pellentesque sit amet porttitor convallis aenean et tortor at risus viverra adipiscing at in tellus integer feugiat scelerisque varius morbi enim nunc faucibus a pellentesque sit amet porttitor";

bool test_uo_strchrnth(void)
{
    bool passed = true;

    passed &= uo_strchrnth(lorem, ' ', 1) == strchr(lorem, ' ');
    passed &= uo_strchrnth(lorem, ' ', 2) == strchr(uo_strchrnth(lorem, ' ', 1) + 1, ' ');

    return passed;
}

bool test_uo_temp_substr(void)
{
    bool passed = true;

    passed &= memcmp(lorem, uo_temp_substr(lorem, 10), 10) == 0;
    passed &= uo_temp_substr(lorem, 10)[11] == '\0';

    return passed;
}

bool test_uo_temp_strcat(void)
{
    bool passed = true;

    passed &= strcmp("lorem ipsum", uo_temp_strcat("lorem ip", "sum")) == 0;

    char *temp = uo_temp_strcat("lorem ip", "sum");
    uo_temp_strcat(temp, " asdf");
    passed &= strcmp("lorem ipsum asdf", temp) == 0;

    return passed;
}

bool test_uo_utf8_append(void)
{
    bool passed = true;

    char utf8[13];
    char *p = utf8;
    p = uo_utf8_append(p, 0x000041); // A
    p = uo_utf8_append(p, 0x0000A5); // ¥
    p = uo_utf8_append(p, 0x0001A9); // Ʃ
    p = uo_utf8_append(p, 0x002615); // ☕
    p = uo_utf8_append(p, 0x010000); // 𐀀

    passed &= strcmp("A¥Ʃ☕𐀀", utf8) == 0;

    return passed;
}

bool test_uo_chrfreq(void)
{
    bool passed = true;

    passed &= uo_chrfreq("/asdf/asdf", 'a', 0) == 2;
    passed &= uo_chrfreq("\"asdf\\\"\"", '\"', '\\') == 2;
    passed &= uo_chrfreq("/asdf/%s/qwert%%20", '%', '%') == 1;
    passed &= uo_chrfreq("/asdf/%s/qwert%%%5s", '%', '%') == 2;
    passed &= uo_chrfreq("/asdf/%s/qwert%%%%5s", '%', '%') == 1;

    return passed;
}

bool test_uo_strdiff(void)
{
    bool passed = true;

    char *str1;
    char *str2;

    str1 = "";
    str2 = "a";
    passed &= uo_strdiff(str1, str1) == NULL;
    passed &= uo_strdiff(str1, str2) == str1 + strlen(str1);

    str1 = "/asdf/asdf";
    str2 = "/asd";
    passed &= uo_strdiff(str1, str1) == NULL;
    passed &= uo_strdiff(str1, str2) == str1 + strlen(str2);

    str1 = "/qwer/{asdf}/zxcv";
    str2 = "/qwer/asdf/zxcv";
    passed &= *uo_strdiff(str1, str2) == '{';

    return passed;
}

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;

    passed &= test_uo_strchrnth();
    passed &= test_uo_temp_substr();
    passed &= test_uo_temp_strcat();
    passed &= test_uo_utf8_append();
    passed &= test_uo_chrfreq();
    passed &= test_uo_strdiff();

    return passed ? 0 : 1;
}
