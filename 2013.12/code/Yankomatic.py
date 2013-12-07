# -*- coding: utf-8 -*-
# <nbformat>3.0</nbformat>

# <codecell>

import nltk
import whoosh
import BeautifulSoup
import urllib2
from pprint import pprint
import cPickle as pickle

# <codecell>

# 0. Syllable analysize, from p2tk
English = {
	'consonants': ['B', 'CH', 'D', 'DH', 'F', 'G', 'HH', 'JH', 'K', 'L', 'M', 'N', 
	'NG', 'P', 'R', 'S', 'SH', 'T', 'TH', 'V', 'W', 'Y', 'Z', 'ZH'],
	'vowels': [ 'AA', 'AE', 'AH', 'AO', 'AW', 'AY', 'EH', 'ER', 'EY', 'IH', 'IY', 'OW', 'OY', 'UH', 'UW'],
	'onsets': ['P', 'T', 'K', 'B', 'D', 'G', 'F', 'V', 'TH', 'DH', 'S', 'Z', 'SH', 'CH', 'JH', 'M',
	'N', 'R', 'L', 'HH', 'W', 'Y', 'P R', 'T R', 'K R', 'B R', 'D R', 'G R', 'F R',
	'TH R', 'SH R', 'P L', 'K L', 'B L', 'G L', 'F L', 'S L', 'T W', 'K W', 'D W', 
	'S W', 'S P', 'S T', 'S K', 'S F', 'S M', 'S N', 'G W', 'SH W', 'S P R', 'S P L',
	'S T R', 'S K R', 'S K W', 'S K L', 'TH W', 'ZH', 'P Y', 'K Y', 'B Y', 'F Y', 
	'HH Y', 'V Y', 'TH Y', 'M Y', 'S P Y', 'S K Y', 'G Y', 'HH W', '']
}

def syllabify(language, word) :
	'''Syllabifies the word, given a language configuration loaded with loadLanguage.
	   word is either a string of phonemes from the CMU pronouncing dictionary set
	   (with optional stress numbers after vowels), or a Python list of phonemes,
	   e.g. "B AE1 T" or ["B", "AE1", "T"]'''
	   
	if type(word) == str :
		word = word.split()
		
	syllables = [] # This is the returned data structure.

	internuclei = [] # This maintains a list of phonemes between nuclei.
	
	for phoneme in word :
	
		phoneme = phoneme.strip()
		if phoneme == "" :
			continue
		stress = None
		if phoneme[-1].isdigit() :
			stress = int(phoneme[-1])
			phoneme = phoneme[0:-1]
		
		if phoneme in language["vowels"] :
			# Split the consonants seen since the last nucleus into coda and onset.
			
			coda = None
			onset = None
			
			# If there is a period in the input, split there.
			if "." in internuclei :
				period = internuclei.index(".")
				coda = internuclei[:period]
				onset = internuclei[period+1:]
			
			else :
				# Make the largest onset we can. The 'split' variable marks the break point.
				for split in range(0, len(internuclei)+1) :
					coda = internuclei[:split]
					onset = internuclei[split:]
					
					# If we are looking at a valid onset, or if we're at the start of the word
					# (in which case an invalid onset is better than a coda that doesn't follow
					# a nucleus), or if we've gone through all of the onsets and we didn't find
					# any that are valid, then split the nonvowels we've seen at this location.
					if " ".join(onset) in language["onsets"] \
					   or len(syllables) == 0 \
					   or len(onset) == 0 :
					   break
			   
			# Tack the coda onto the coda of the last syllable. Can't do it if this
			# is the first syllable.
			if len(syllables) > 0 :
				syllables[-1][3].extend(coda)
			
			# Make a new syllable out of the onset and nucleus.
			syllables.append( (stress, onset, [phoneme], []) )
				
			# At this point we've processed the internuclei list.
			internuclei = []

		elif not phoneme in language["consonants"] and phoneme != "." :
			raise ValueError, "Invalid phoneme: " + phoneme
			
		else : # a consonant
			internuclei.append(phoneme)
	
	# Done looping through phonemes. We may have consonants left at the end.
	# We may have even not found a nucleus.
	if len(internuclei) > 0 :
		if len(syllables) == 0 :
			syllables.append( (None, internuclei, [], []) )
		else :
			syllables[-1][3].extend(internuclei)

	return syllables

def stringify(syllables) :
	'''This function takes a syllabification returned by syllabify and
	   turns it into a string, with phonemes spearated by spaces and
	   syllables spearated by periods.'''
	ret = []
	for syl in syllables :
		stress, onset, nucleus, coda = syl
		if stress != None and len(nucleus) != 0 :
			nucleus[0] += str(stress)
		ret.append(" ".join(onset + nucleus + coda))
	return " . ".join(ret)

# <codecell>

def expand_dictionary(food_words):
    full_food_words = set()
    
    lemmatizer = nltk.WordNetLemmatizer()
    
    for f in food_words:
        full_food_words.add(f)
        full_food_words.add(lemmatizer.lemmatize(f))
    return sorted(list(full_food_words))

# <codecell>

arpabet = nltk.corpus.cmudict.dict()

# <codecell>

# 1. Get food words
# Food url is http://www.food.com/library/all.zsp
def get_food_data():
    f = urllib2.urlopen('http://www.food.com/library/all.zsp')
    raw_food_data = BeautifulSoup.BeautifulSoup(f)
    f.close()
    return raw_food_data

# <codecell>

def get_foods(raw_data):
    content = raw_data.findAll('div', 'content')[0]
    return expand_dictionary([item.findAll('a')[0]['title'].lower() for item in content.findAll('li')])

# <codecell>

def analyze_rhyme(phonemes):
    ''' Returns the stress pattern, and the rhyme scheme of the last syllable '''
    syllables = syllabify(English, ' '.join(phonemes))
    
    stresses = [int(x[0]>0) for x in syllables]
    tail     = filter(lambda x: len(x) > 0, syllables[-1][1:])
    
    # Crunch the tail?
    # Only if the last phoneme is a vowel and the first is a consonant
    if ' '.join(tail[0]) in English['onsets'] and ''.join(tail[-1]) not in English['vowels']:
        tail = tail[1:]
    
    _tail = []
    for t in tail:
        _tail.extend(t)
    return stresses, _tail

# <codecell>

# Use the P2TK syllabifier
# Count syllables in each word
# word => (tail, # syllables)
# 'tail' is the last syllable minus the onset

# <codecell>

raw_food_data = get_food_data()

# <codecell>

food_strings = get_foods(raw_food_data)

# <codecell>

# stress -> ending -> list of words
food_mapping = {}
for s in food_strings:
    # tokenize s
    #s = nltk.tokenize.word_tokenize(s)
    
    if s in arpabet:
        for p in arpabet[s]:
            stresses, tail = analyze_rhyme(p)
            key_str = ''.join(map(str, stresses))
            key_end = '_'.join(tail)
            
            if key_str not in food_mapping:
                food_mapping[key_str] = {}
                
            if key_end not in food_mapping[key_str]:
                food_mapping[key_str][key_end] = set()
                
            food_mapping[key_str][key_end].add(s)

# <codecell>

# 3. Build a query interface
def rhyme_searcher(mapping, query):
    query = query.lower()
    
    results = set()
    if query in arpabet:
        for p in arpabet[query]:
            stresses, tail = analyze_rhyme(p)
            key_str = ''.join(map(str, stresses))
            key_end = '_'.join(tail)
    
            if key_str in mapping:
                if key_end in mapping[key_str]:
                    results.update(mapping[key_str][key_end])
                    
    return results

# <codecell>

def string_query(food_mapping, query):
    
    # Tokenize the query
    tokens = nltk.tokenize.word_tokenize(query)
    
    # Part-of-speech tag
    tags = nltk.pos_tag(tokens)
    
    results = set()
    
    for word in filter(lambda x: x[-1][0] == 'N', tags):
        
        replacements = rhyme_searcher(food_mapping, word[0])
        
        if len(replacements) == 0:
            continue
        
        for r in replacements:
            results.add(query.replace(word[0], r))
            
    # Return everything except the original query
    return sorted(list(results - set([query])))

# <codecell>

# Save the language and dictionary files
with open('/home/bmcfee/git/musichacks/2013.12/data/model.pickle', 'w') as f:
    pickle.dump([English, food_mapping], f)

# <codecell>

string_query(food_mapping, "robot rock")

