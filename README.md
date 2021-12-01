# Fast Attribute-based encryption

## Final Year Project
### &emsp; B.Tech, &ensp;NIT Calicut
### &emsp; Batch: &nbsp;2018 - 2022
<br>

>
> | Group 25  |                           |
> | :---      |:---                       |
> | B180008CS | Alan Jojo                 |
> | B180363CS | Atar Mohammad Umar Farooq |
> | B180347CS | Emmanuel Marian Mathew    |

<br>

The pre-existing implementation of FAME (given in [Attribute-based Encryption](https://github.com/sagrawal87/ABE "Click to go to the repository")) is written in Python 2.7 and uses the CHARM library making it obsolete and also computationally slower.

As proposed for further improvement by the paper [FAME: Fast Attribute-based Message Encryption](https://eprint.iacr.org/2017/807 "Click to view the research paper") by Shashank Agrawal and Melissa Chase, we try to build a comprehensive implementation of the FAME scheme using the **C++ programming language** in this project.

With our work, we are aiming to decrease the time required to set up, generate keys, encrypt and decrypt with the power of C++ and also a few possible improvements from the current implementation.

<br>

To run the code, use the following commands on the Linux bash terminal:

    g++ policytree.cpp -o policytree
    ./policytree