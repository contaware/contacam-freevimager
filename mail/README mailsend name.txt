Up to ContaCam 9.9.22 the original mailsend.exe written in 32-bit C was 
used. Since ContaCam 9.9.23 it has been replaced by the 32-bit 
mailsend-go.exe written in Golang. The mailsend-go.exe executable was 
renamed to mailsend.exe to allow all ContaCam installers (old and new 
ones) to easily find the mailer process and kill it before copying the 
files. This makes ContaCam upgrading and downgrading smooth and 
straightforward.

Attention: do not exchange the C and the Golang mailsend, they have 
           incompatible command line options!
