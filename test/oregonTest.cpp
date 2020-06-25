/*
 * This file is part of the TinySensor distribution (https://github.com/arcadien/TinySensor)
 *
 * Copyright (c) 2019 Aurélien Labrosse
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <unity.h>
#include <protocol/Oregon.h>
void setUp(void)
{
}

void tearDown(void)
{
}

void Expect_Oregon_Buffers_to_be_correctly_sized_in_mode0()
{
    Oregon<MODE_0> oregon;
    TEST_ASSERT_EQUAL(Oregon<MODE_0>::BUFF_SIZE[MODE_0], sizeof(oregon._oregonMessageBuffer));
}

void Expect_Oregon_Buffers_to_be_correctly_sized_in_mode1()
{
    Oregon<MODE_1> oregon1;
    TEST_ASSERT_EQUAL(Oregon<MODE_1>::BUFF_SIZE[MODE_1], sizeof(oregon1._oregonMessageBuffer));
}

void Expect_Oregon_Buffers_to_be_correctly_sized_in_mode2()
{
    Oregon<MODE_2> oregon2;
    TEST_ASSERT_EQUAL(Oregon<MODE_2>::BUFF_SIZE[MODE_2], sizeof(oregon2._oregonMessageBuffer));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(Expect_Oregon_Buffers_to_be_correctly_sized_in_mode0);
    RUN_TEST(Expect_Oregon_Buffers_to_be_correctly_sized_in_mode1);
    RUN_TEST(Expect_Oregon_Buffers_to_be_correctly_sized_in_mode2);

    return UNITY_END();
}
