#include "uo_stack.h"

#include <stdbool.h>

const char *lorem = "elementum nibh tellus molestie nunc non blandit massa enim nec dui nunc mattis enim ut tellus elementum sagittis vitae et leo duis ut diam quam nulla porttitor massa id neque aliquam vestibulum morbi blandit cursus risus at ultrices mi tempus imperdiet nulla malesuada pellentesque elit eget gravida cum sociis natoque penatibus et magnis dis parturient montes nascetur ridiculus mus mauris vitae ultricies leo integer malesuada nunc vel risus commodo viverra maecenas accumsan lacus vel facilisis volutpat est velit egestas dui id ornare arcu odio ut sem nulla pharetra diam sit amet nisl suscipit adipiscing bibendum est ultricies integer quis auctor elit sed vulputate mi sit amet mauris commodo quis imperdiet massa tincidunt nunc pulvinar sapien et ligula ullamcorper malesuada proin libero nunc consequat interdum varius sit amet mattis vulputate enim nulla aliquet porttitor lacus luctus accumsan tortor posuere ac ut consequat semper viverra nam libero justo laoreet sit amet cursus sit amet dictum sit amet justo donec enim diam vulputate ut pharetra sit amet aliquam id diam maecenas ultricies mi eget mauris pharetra et ultrices neque ornare aenean euismod elementum nisi quis eleifend quam adipiscing vitae proin sagittis nisl rhoncus mattis rhoncus urna neque viverra justo nec ultrices dui sapien eget mi proin sed libero enim sed faucibus turpis in eu mi bibendum neque egestas congue quisque egestas diam in arcu cursus euismod quis viverra nibh cras pulvinar mattis nunc sed blandit libero volutpat sed cras ornare arcu dui vivamus arcu felis bibendum ut tristique et egestas quis ipsum suspendisse ultrices gravida dictum fusce ut placerat orci nulla pellentesque dignissim enim sit amet venenatis urna cursus eget nunc scelerisque viverra mauris in aliquam sem fringilla ut morbi tincidunt augue interdum velit euismod in pellentesque massa placerat duis ultricies lacus sed turpis tincidunt id aliquet risus feugiat in ante metus dictum at tempor commodo ullamcorper a lacus vestibulum sed arcu non odio euismod lacinia at quis risus sed vulputate odio ut enim blandit volutpat maecenas volutpat blandit aliquam etiam erat velit scelerisque in dictum non consectetur a erat nam at lectus urna duis convallis convallis tellus id interdum velit laoreet id donec ultrices tincidunt arcu non sodales neque sodales ut etiam sit amet nisl purus in mollis nunc sed id semper risus in hendrerit gravida rutrum quisque non tellus orci ac auctor augue mauris augue neque gravida in fermentum et sollicitudin ac orci phasellus egestas tellus rutrum tellus pellentesque eu tincidunt tortor aliquam nulla facilisi cras fermentum odio eu feugiat pretium nibh ipsum consequat nisl vel pretium lectus quam id leo in vitae turpis massa sed elementum tempus egestas sed sed risus pretium quam vulputate dignissim suspendisse in est ante in nibh mauris cursus mattis molestie a iaculis at erat pellentesque adipiscing commodo elit at imperdiet dui accumsan sit amet nulla facilisi morbi tempus iaculis urna id volutpat lacus laoreet non curabitur gravida arcu ac tortor dignissim elementum nibh tellus molestie nunc non blandit massa enim nec dui nunc mattis enim ut tellus elementum sagittis vitae et leo duis ut diam quam nulla porttitor massa id neque aliquam vestibulum morbi blandit cursus risus at ultrices mi tempus imperdiet nulla malesuada pellentesque elit eget gravida cum sociis natoque penatibus et magnis dis parturient montes nascetur ridiculus mus mauris vitae ultricies leo integer malesuada nunc vel risus commodo viverra maecenas accumsan lacus vel facilisis volutpat est velit egestas dui id ornare arcu odio ut sem nulla pharetra diam sit amet nisl suscipit adipiscing bibendum est ultricies integer quis auctor elit sed vulputate mi sit amet mauris commodo quis imperdiet massa tincidunt nunc pulvinar sapien et ligula ullamcorper malesuada proin libero nunc consequat interdum varius sit amet mattis vulputate enim nulla aliquet porttitor lacus luctus accumsan tortor posuere ac ut consequat semper viverra nam libero justo laoreet sit amet cursus sit amet dictum sit amet justo donec enim diam vulputate ut pharetra sit amet aliquam id diam maecenas ultricies mi eget mauris pharetra et ultrices neque ornare aenean euismod elementum nisi quis eleifend quam adipiscing vitae proin sagittis nisl rhoncus mattis rhoncus urna neque viverra justo nec ultrices dui sapien eget mi proin sed libero enim sed faucibus turpis in eu mi bibendum neque egestas congue quisque egestas diam in arcu cursus euismod quis viverra nibh cras pulvinar mattis nunc sed blandit libero volutpat sed cras ornare arcu dui vivamus arcu felis bibendum ut tristique et egestas quis ipsum suspendisse ultrices gravida dictum fusce ut placerat orci nulla pellentesque dignissim enim sit amet venenatis urna cursus eget nunc scelerisque viverra mauris in aliquam sem fringilla ut morbi tincidunt augue interdum velit euismod in pellentesque massa placerat duis ultricies lacus sed turpis tincidunt id aliquet risus feugiat in ante metus dictum at tempor commodo ullamcorper a lacus vestibulum sed arcu non odio euismod lacinia at quis risus sed vulputate odio ut enim blandit volutpat maecenas volutpat blandit aliquam etiam erat velit scelerisque in dictum non consectetur a erat nam at lectus urna duis convallis convallis tellus id interdum velit laoreet id donec ultrices tincidunt arcu non sodales neque sodales ut etiam sit amet nisl purus in mollis nunc sed id semper risus in hendrerit gravida rutrum quisque non tellus orci ac auctor augue mauris augue neque gravida in fermentum et sollicitudin ac orci phasellus egestas tellus rutrum tellus pellentesque eu tincidunt tortor aliquam nulla facilisi cras fermentum odio eu feugiat pretium nibh ipsum consequat nisl vel pretium lectus quam id leo in vitae turpis massa sed elementum tempus egestas sed sed risus pretium quam vulputate dignissim suspendisse in est ante in nibh mauris cursus mattis molestie a iaculis at erat pellentesque adipiscing commodo elit at imperdiet dui accumsan sit amet nulla facilisi morbi tempus iaculis urna id volutpat lacus laoreet non curabitur gravida arcu ac tortor dignissim convallis aenean et tortor at risus viverra adipiscing at in tellus integer feugiat scelerisque varius morbi enim nunc faucibus a pellentesque sit amet porttitor convallis aenean et tortor at risus viverra adipiscing at in tellus integer feugiat scelerisque varius morbi enim nunc faucibus a pellentesque sit amet porttitor";

int main(
    int argc, 
    char const **argv)
{
    bool passed = true;

    uo_stack *stack = uo_stack_create(0x10);

    passed &= stack->capacity == 0x10;

    for (size_t i = 0; i < 0x1000; ++i)
    {
        passed &= uo_stack_count(stack) == i;
        uo_stack_push(stack, lorem + i);
    }

    passed &= uo_stack_index(stack, 0) == lorem + 0;
    passed &= uo_stack_index(stack, 0x100) == lorem + 0x100;
    passed &= uo_stack_index(stack, -1) == lorem + uo_stack_count(stack) - 1;
    passed &= uo_stack_index(stack, -1) == uo_stack_peek(stack);

    for (size_t i = 0; i < 0x1000; ++i)
    {
        passed &= uo_stack_count(stack) == 0x1000 - i;
        passed &= uo_stack_pop(stack) == (void *)(lorem + 0x1000 - 1 - i);
    }

    uo_stack_push_arr(stack, (void **)lorem, 0x10);
    passed &= uo_stack_count(stack) == 0x10;

    uo_stack_push_arr(stack, (void **)lorem + 0x10, 0x10);
    passed &= uo_stack_count(stack) == 0x20;

    uo_stack_insert_arr(stack, 0x10, (void **)lorem + 0x20, 0x10);
    passed &= uo_stack_count(stack) == 0x30;

    uo_stack_insert_arr(stack, -0x10, (void **)lorem + 0x30, 0x10);
    passed &= uo_stack_count(stack) == 0x40;

    for (size_t i = 0; i < 0x10; ++i)
        passed &= uo_stack_index(stack, i) == ((void **)lorem)[i];

    for (size_t i = 0x10; i < 0x20; ++i)
        passed &= uo_stack_index(stack, i) == ((void **)lorem)[i + 0x10];

    for (size_t i = 0x20; i < 0x30; ++i)
        passed &= uo_stack_index(stack, i) == ((void **)lorem)[i + 0x10];

    for (size_t i = 0x30; i < 0x40; ++i)
        passed &= uo_stack_index(stack, i) == ((void **)lorem)[i - 0x20];

    for (size_t i = 0; i < 0x10; ++i)
        passed &= uo_stack_pop(stack) == ((void **)lorem)[0x20 - 1 - i];

    for (size_t i = 0; i < 0x10; ++i)
        passed &= uo_stack_pop(stack) == ((void **)lorem)[0x40 - 1 - i];

    for (size_t i = 0; i < 0x10; ++i)
        passed &= uo_stack_pop(stack) == ((void **)lorem)[0x30 - 1 - i];

    for (size_t i = 0; i < 0x10; ++i)
        passed &= uo_stack_pop(stack) == ((void **)lorem)[0x10 - 1 - i];

    passed &= uo_stack_count(stack) == 0;

    uo_stack_insert(stack, 0x0, lorem + 0x0);
    uo_stack_insert(stack, 0x0, lorem + 0x1);
    uo_stack_insert(stack, 0x0, lorem + 0x2);
    uo_stack_insert(stack, 0x0, lorem + 0x3);

    passed &= uo_stack_count(stack) == 4;

    passed &= uo_stack_pop(stack) == lorem + 0x0;
    passed &= uo_stack_pop(stack) == lorem + 0x1;
    passed &= uo_stack_pop(stack) == lorem + 0x2;
    passed &= uo_stack_pop(stack) == lorem + 0x3;

    passed &= uo_stack_count(stack) == 0;

    return passed ? 0 : 1;
}
