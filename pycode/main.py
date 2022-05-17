#!/bin/python
from msp import MSP
from policytree import PolicyParser
scheme = '((((ONE OR THREE) AND (FOUR AND FIVE)) OR ((TWO AND FOUR) AND (ONE OR TWO))) AND((ONE OR TWO) AND ((THREE OR FIVE) AND (TWO OR FOUR))))'
msp_obj = MSP('group_obj')
parser = PolicyParser()
policy_obj = parser.parse(scheme)
attr = ['ONE','TWO','THREE','FOUR']
print(msp_obj.prune(policy_obj, attr))
