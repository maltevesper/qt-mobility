TEMPLATE = subdirs
TARGET = 

CONFIG += ordered

SUBDIRS += performance \   
           ut_transformcontactdata \
           ut_cntsymbianengine \
           ut_cntfiltering \
           ut_cntrelationship
           