# YXX
YAML <-> XML eXchange  
Not to be confused with *Abbotsford International Airport*

- [YXX](#yxx)
- [Motivation](#motivation)
- [Specification](#specification)
  - [Introduction](#introduction)
  - [Tags and Attributes](#tags-and-attributes)
    - [Anatomy of YXX tag objects](#anatomy-of-yxx-tag-objects)
  - [Text rendering](#text-rendering)
  - [YAML \<-\> XML feature parity](#yaml---xml-feature-parity)
    - [Native YAML features](#native-yaml-features)
      - [Anchors and Aliases in YXX](#anchors-and-aliases-in-yxx)
    - [YXX dealing with nested arrays](#yxx-dealing-with-nested-arrays)
    - [YXX dealing with multiline text](#yxx-dealing-with-multiline-text)
    - [YXX dealing with non-string attribute values](#yxx-dealing-with-non-string-attribute-values)
      - [String concatenation](#string-concatenation)
  - [Special XML nodes](#special-xml-nodes)
    - [Comments](#comments)
    - [Processing Instruction node](#processing-instruction-node)
    - [Declaration node](#declaration-node)
    - [Doctype node](#doctype-node)
- [Bitter Recompense](#bitter-recompense)

A convention for (almost)<sup>1</sup> lossless YAML <-> XML mapping, meaning that data expected as XML can have a 1:1 YAML representation without the loss of XML features. Its syntax should feel natural and should not have arbitrary symbols with special meaning (almost)<sup>2</sup>.

Check out the couple of examples in this repository, if not in the mood to read walls of text.

1. Almost because YXX embraces YAML anchors and aliases in ways which XML cannot support. So when a YXX document, using these extra features, is converted to their XML equivalent, and then that XML equivalent converted back to YXX, the extra features will be gone.
2. Except couple of things:
   1. The "null" or "empty" key representing the body of an XML tag with attributes written as block scalar
   2. Comments are represented by special key `//` ([see Comments](#comments))
   3. Declaration node is represented by special key `<?xml` ([see Declaration node](#declaration-node))
   4. Processing instruction nodes are represented by `<?` prefix ([see Processing Instruction node](#processing-instruction-node))
   5. Doctype nodes are represented by `<!` prefix ([see Doctype node](#doctype-node))

# Motivation

My personal grievence with XML is its usage in Unreal Engine, specifically speaking of DSL's expressed in XML notation like UPL or BuildGraph, which both could have benefited from a much more expressive, off-the-shelf language also used by Unreal Engine itself in many places to define build automations and testing scenarios (<sup>khm</sup>C#<sup>khm</sup>). But I digress an entire industry is now using those and who am I, or even who am anybody individual to speak against the infinite wisdom of the industry.

So instead of coming up with an actually well (intended to be) designed DSL for individual features they represent, I'm doodling with the idea of making their existing structure more bearable to look at with emulating XML features with the power of plain non-modified YAML and nothing else added, using the XML representation only as an intermediary.

# Specification

## Introduction

Take a very simple, a demure XML document, let's say a C# project (not a working one though)

```xml
<Project>
  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>net8.0</TargetFramework>
    <langversion>12</langversion>
    <RootNamespace></RootNamespace>
  </PropertyGroup>
</Project>
```

Mapping this to YAML is trivial (and the simplest case for the YXX convention, and the trivial output of many XML to YAML converters out there)

```yaml
- Project:
  - PropertyGroup:
    - OutputType: Exe
    - TargetFramework: net8.0
    - LangVersion: 12
    - RootNameSpace:
```

As you can notice tags are mapped to objects in arrays (including the root node for conistency), because an XML tag's body can contain any number of the same tag, however YAML objects must have unique keys. The first key of these objects denote their tag name, and if its value is not an object that will represent their body. This is the simplified notation in YXX, if that would be it I wouldn't write this entire thing.

Things get slightly more complicated when tag attributes are introduced. Now XML attributes also must be unique so those can easily map to YAML objects, however YAML doesn't have a syntactical separation for "object attributes", "object type" and "object contents" like how XML does...

## Tags and Attributes

Or is there? [cue vsauce music]

There are two things about YAML keys which are not trivially thought of:

1. Keys (or even values) are optional
2. Keys can be any scalar type including of course strings, numbers, booleans, null **(!)**, arrays **(?)** or objects ***(??)***

**null** is the default one if a key is not specified, and as null is only equal to itself an object can only have one null key (this will be important later)

These keys can even span multiple lines in which case they need to use explicit key-value notation:

```yaml
# flow scalar keys doesn't need explicit "?:" pair
{ foo: bar, hello: world }: myvalue
[ array, of, things ]: myvalue

# or key can be even block scalar
? - my
  - array
  - key
  - { with: flow scalar objects,
      even on: multiple lines look at this
    }
: myvalue
```

This yaml feature could actually map nicely to specify both a list of attributes and a direct value for an entity in the document, however one part is still missing, the tag name.

Fear not though, writing explicit key-value notation is very cumbersome, may look even uglier than XML, and defeats the purpose of being "human friendly". For these reasons YXX should never require the author to write documents with it.

### Anatomy of YXX tag objects

Generally speaking in YXX, tags can be declared the following ways:

A tag without attributes can simply declare its body with a sequence scalar
```yaml
- SimpleTag:
  - ...
```

A tag with direct attributes object scalar and with optional directly descendant body
```yaml
- MultilineLeafTag:
    Foo: bar
    Hello: World

- SingleLineLeafTag: { Foo: bar, Hello: World }

- MultilineTag:
    Foo: bar
    Hello: World
  :
  - ...

- DoubleLineTag: { Foo: bar, Hello: World }
  :
  - ...
```

Or this:
```yaml
- SingleLineTag: { Foo: bar, Hello: World }:
  - ...
```

Notice how in practice the `:` colon just went from the bottom of the tag to the end of the tag, but in terms of YAML that is a significant and a mounthful difference:

A tag object containing a single member, which key is representing the tag name, which value is another object with one member, which has a complex object for key, representing the attributes and the value of that single member with the complex key represents the body of the tag. A diagram worths a thousand words.

```yaml
#                         T: TAG OBJECT                          
# |--------------------------------------------------------------|
# |                         AB: ATTRIBUTES AND BODY PAIR         |
# |                 |-------------------------------------------||
# |                 |     KA: COMPLEX KEY (OBJECT)              ||
# |                 ||--------------------------------|         ||
# |                 ||         A: ATTRIBUTES          |         ||
# |                 ||   |------------------------|   |         ||
-   SingleLineTag :    {   Foo: bar, Hello: World   }   : - ... 
#                                                       | |
#                                                       | Sequence scalar actually belonging to KA
#                                                       | instead of being direct member of T
#                                                       |
#                                                       Makes KA the key
```


For example this XML on the left is expressed with YAML on the right:

<table>
<tr>
<td>
  
```xml
<Project Sdk="Microsoft.NET.Sdk">
  <ItemGroup>
    <PackageReference Include="md.Nuke.Unreal" Version="2.1.1" />
    <PackageReference Include="Nuke.Common" Version="8.1.4" />
  </ItemGroup>
</Project>
```

</td>
<td>

```yaml
- Project: { Sdk: Microsoft.NET.Sdk }:
  - ItemGroup:
    - PackageReference: { Include: md.Nuke.Unreal, Version: 2.1.1 }
    - PackageReference: { Include: Nuke.Common, Version: 8.1.4 }
```

</td>
</tr>
</table>

To simplify talking about these entities we will assign them YXX specific names:

```yaml
# tag object
# tag key: tag scalar (not body!)
- Project:
    # attributes objectx
    # attribute key: attribute value
    Sdk: Microsoft.NET.Sdk

  # extra named tag members which are all ignored (after the first one) with their entire sub structure
  A: ignored during conversion
  B: also ignored during conversion
  foobar: also ignored during conversion

  # directly descendant body key (an empty key which has a non-object value)
  :
  # tag body

  # simple tag object
  - ItemGroup: # tag body

    - PackageReference: etc...

# OR
# tag object
# tag key: tag scalar (not body!)
#          | complex attr. object   |: deferred body scalar
- Project: { Sdk: Microsoft.NET.Sdk }:
  - ... # this sequence actually belongs to the object member keyed with the flow mapping
        # containing the attributes
```

Much like how XML closing tag can be shortened, the contents under empty-key is also optional (see how that's missing in `PackageReference`)

When tags with attributes doesn't have a body key, OR doesn't have a body scalar, OR when the tag body scalar is null (`""` empty string is not empty value neither null!) the XML tag is rendered with self closing slash. That means

<table>
<tr>
<td>

```yaml
- Project:
  - Foo:
```

</td>
<td> -> </td>
<td>

```xml
<Project>
    <Foo />
</Project>
```

</td>
</tr>
</table>

## Text rendering

Non-object array items are simply rendered as text nodes inside the resulting XML, so

<table>
<tr>
<td>

```yaml
- div:
  - This is plain text
  - p: This is a paragraph
  - Closing thoughts
```

</td>
<td> -> </td>
<td>

```xml
<div>
    This is plain text
    <p>This is a paragraph</p>
    Closing thoughts
</div>
```

</td>
</tr>
</table>

This is a good segue into the **Markup** notion of XML and simply put how there's no natural feeling syntax for replicating it in YAML (well as the name suggest, it ain't being a markup language 😉 ). So this feature of XML can be only mapped with an awkward flow scalar array maybe

```xml
<p>we have <b>rich <i>text</i></b> markers</p>
```

may be replicated with

```yaml
- p: [we have, b: [ rich, i: text ], markers]
```

Eeew. Don't do that.

Well YXX doesn't try to replace XML where XML shines anyway so moving on. Speaking of plain text or any string value, all text is escaped for xml so even writing

<table>
<tr>
<td>

```yaml
- p: we have <b>rich <i>text</i></b> markers
```

</td>
<td>

```xml
<p>we have &lt;b&gt;rich &lt;i&gt;text&lt;/i&gt;&lt;/b&gt; markers</p>
```

</td>
</tr>
</table>

This means a kind of "mixed" XML/YAML mode is not allowed. The reasoning behind this is YXX is designed to be used in rigid systems where XML has been decided to represent structured data, where syntactical markup doesn't make much sense. In those scenarios it is more annoying to deal with XML encoded text than the possibility of inline markup, and imagine the absurdity that you'd have to XML encode your text in YAML.

As a cherry-picked counter example where YXX can shine instead of its original XML form is in BuildGraph which has a [`<WriteTextFile>`](https://github.com/microdee/YXX/blob/main/Convention/Unreal/BuildGraph.xml.yml#L136) tag where the contents of the file is meant to be specified in an attribute. In one line. I've seen people using it dumping entire powershell scripts in there. These are truly dark times we live in.

## YAML \<-\> XML feature parity

YAML brings some both neat and/or controversial features XML doesn't have (it's easy to compete when one has two decades of head-start over the other), like anchors, aliases and object merging. YAML also can express some structures impossible to purely duplicate in XML without special tags, which YXX may utilize when converting from YAML to XML.

Both cases are one directional conversions only, once a YXX document using these features converted to XML, they will be unrolled of course and their original structure will lose to constraints of XML. Converting that XML back to YXX cannot preserve these features.

### Native YAML features

#### Anchors and Aliases in YXX

`&anchors`, `*aliases` and `<<: *object_merging` are advanced features of YAML aimed at mitigating repetition at the data structure level. These are sometimes criticized for "doing way to much for an object notation, and further complicates writing parsers". However in a world where XML DSL's may not have the courtesy to provide code-reusability features I personally highly welcome these in YAML, especially when most bigger YAML libraries support these pretty consistently.

Obvious benefit is when a repeating set of node is in the XML tree, in YXX one can write the following

```yaml
- Root:
  - &repeated_node MyNode: { foo: bar, there: are a lot of, attributes: repeating }
  - *repeated_node
  - *repeated_node
  - *repeated_node
  # ad infinitum
```

Of course anchors can be put in front of any value and aliases can be put similarly anywhere so maybe a more realistic example would be

```yaml
- Root:
  - MyNode: &common_attributes { foo: bar, there: are a lot of, attributes: repeating }
  - SomeOtherNode: *common_attributes
  - MyTag: *common_attributes
  - OverrideOneAttribute: { <<: *common_attributes, there: you are }
```

However why `MyNode` has the responsibility of defining these common attributes when maybe conceptually speaking, it doesn't have any more significance than the other tags in there? Remember some paragraphs ago that in a tag object only the first key and the empty key is considered? Those can be used to declare these anchors at a nice central places without polluting the XML output.

```yaml
- Root: # no attributes
  vars:
    - &common_attributes { foo: bar, there: are a lot of, attributes: repeating }
  :
  - MyNode: *common_attributes
  - SomeOtherNode: *common_attributes
  - MyTag: *common_attributes
  - OverrideOneAttribute: { <<: *common_attributes, there: you are }
```

Again anchors can have primitive values or even arrays so an entire structure can be repeated with them

```yaml
# an imaginary CI/CD pipeline
- MyPipeline: # no attributes
  # the character / has no significance here, it just helps separating ignored keys
  /artifacts:
    - &artifact_name MyProgram
  /commonSteps:
    - &prepare_task Task: { Name: Prepare }:
      - Clone: { ... }
      - Pwsh: { ... }
      # ...
    - &cleanup_task Task: { Name: Prepare }:
      - Pwsh: { ... }
  :

  - Job: { Id: foo, Produces: &artifact_name }:
    - &prepare_task
    - Task: { Name: build }:
      - # steps...
    - &cleanup_task

  - Job: { Id: bar, Consumes: &artifact_name }:
    - &prepare_task
    - Task: { Name: test }:
      - # steps...
    - &cleanup_task
```

### YXX dealing with nested arrays

In regular yaml one is free to do nested arrays

```yaml
output:
  - [a, b, c]
  - [d, e, f]
  - [[1, 2], [3, 4], [5, 6]]
  - - aaa
    - bbb
    - ccc
  - - 111
    - 222
    - 333
```

This doesn't really map to any XML syntax, however nested arrays give an opportunity to work around the YAML limitation that arrays cannot be merged/concatenated like objects can. However one can still place them next to each other in an array:

```yaml
predefined sequences:
  - &abc [a, b, c]
  - &def [d, e, f]
  - &vectors
    - [1, 2]
    - [3, 4]
    - [5, 6]
output:
  - *abc
  - *def
  - *vectors

# expands to the first half of the previous figure
```

When converting from a nested array structure like this from YAML to XML, YXX flatten nested arrays into the first layer. This allows to have a sequence of tags stored under one anchor, rather than needing an encompassing tag:

<table>
<tr>
<td>

```yaml
- Schedule:
  /vars:
    - &work_day
      - WakeUp:
      - Coffee:
      - GoToWork:
      - Code:
      - GoHome:
      - Sleep:
    - &weekend
      - WakeUp:
      - VideoGames:
      - Sleep:
  :
  - *work_day
  - *work_day
  - *weekend
  - *weekend
```

</td>
<td> -> </td>
<td>

```xml
<Schedule>
  <WakeUp />
  <Coffee />
  <GoToWork />
  <Code />
  <GoHome />
  <Sleep />
  <WakeUp />
  <Coffee />
  <GoToWork />
  <Code />
  <GoHome />
  <Sleep />
  <WakeUp />
  <VideoGames />
  <Sleep />
  <WakeUp />
  <VideoGames />
  <Sleep />
</Schedule>
```

</td>
</tr>
</table>

### YXX dealing with multiline text

YXX converters should respect yaml multiline text everywhere and let the underlying YAML parser handle new-line, indentation and leading/trailing spaces behavior (hopefully following the yaml specifications). Plain text nodes in tag bodies are XML encoded but left with new-lines untouched. However multiline attribute values will have the parsed newlines XML encoded.

```yaml
- Root:
  - Curse:
      ButWhy: | # too much stuff for an attribute but deadline is here and we need to please the stakeholders
        Irure est minim reprehenderit dolor deserunt proident incididunt consectetur. Qui culpa
        excepteur nisi. Ad culpa veniam sit mollit laboris ea reprehenderit voluptate sunt qui
        proident. Est commodo adipisicing ex. Fugiat incididunt amet incididunt dolore.
    :
    - | # damn it even has inner text, every day we stray further from god...
      Aliqua magna ullamco eiusmod. Est non deserunt est ut commodo nisi nostrud proident mollit
      commodo. Tempor laborum ullamco eiusmod incididunt non nostrud nulla est veniam do. Lorem
      nostrud magna quis minim Lorem qui cillum labore et veniam laborum. Non irure ex minim
      voluptate commodo. Pariatur commodo velit consectetur nostrud id ea labore. Lorem Lorem
      nostrud irure elit tempor tempor laborum eiusmod officia deserunt est consectetur aute.
      Lorem consequat cillum enim ipsum consequat elit in ex non magna culpa occaecat tempor
      tempor cillum.
```

<details><summary>Produces the following XML</summary>

```xml
<Root>
  <Curse ButWhy="Irure est minim reprehenderit dolor deserunt proident incididunt consectetur. Qui culpa&#13;excepteur nisi. Ad culpa veniam sit mollit laboris ea reprehenderit voluptate sunt qui&#13;proident. Est commodo adipisicing ex. Fugiat incididunt amet incididunt dolore.">
    Aliqua magna ullamco eiusmod. Est non deserunt est ut commodo nisi nostrud proident mollit
    commodo. Tempor laborum ullamco eiusmod incididunt non nostrud nulla est veniam do. Lorem
    nostrud magna quis minim Lorem qui cillum labore et veniam laborum. Non irure ex minim
    voluptate commodo. Pariatur commodo velit consectetur nostrud id ea labore. Lorem Lorem
    nostrud irure elit tempor tempor laborum eiusmod officia deserunt est consectetur aute.
    Lorem consequat cillum enim ipsum consequat elit in ex non magna culpa occaecat tempor
    tempor cillum.
  </Curse>
</Root>
```

</details>

### YXX dealing with non-string attribute values

In XML tag attributes can only have string values. YAML obviously doesn't pose such limitation, so YXX attempts to respect that freedom by simply converting simple scalars to string (null or empty is `""`) and reducing objects or sequence values if they're understandable in this context. In this regard YXX introduces some arbitrary transformation to map complex data into single line strings but I promise it's for the greater good.

#### String concatenation

**Plain arrays are concatenated into a continuous string without any delimiter**

<table>
<tr>
<td>

```yml
- Tag:
    Attr:
      - Hello
      - World
```

</td>
<td> -> </td>
<td>

```xml
<Tag Attr="HelloWorld" />
```

</td>
</tr>
</table>

**Nested arrays are flattened as with tag bodies**

<table>
<tr>
<td>

```yml
- Tag:
    Attr:
      - - Foo
        - Bar
      - - Hello
        - World
```

</td>
<td> -> </td>
<td>

```xml
<Tag Attr="FooBarHelloWorld" />
```

</td>
</tr>
</table>

**A delimiter can be specified with a specially formatted object**

<table>
<tr>
<td>

```yml
- Tag:
    Attr:
      ' ':
      - Hello
      - World
```

</td>
<td> -> </td>
<td>

```xml
<Tag Attr="Hello World" />
```

</td>
</tr>
</table>

**Consistently flatten nested arrays**

<table>
<tr>
<td>

```yml
- Tag:
    Attr:
      ' ':
      - - Foo
        - Bar
      - - Hello
        - World
```

</td>
<td> -> </td>
<td>

```xml
<Tag Attr="Foo Bar Hello World" />
```

</td>
</tr>
</table>

**Combine delimiters**

<table>
<tr>
<td>

```yml
- Tag:
    Attr:
      ;:
      - ,:
        - Foo
        - Bar
      - ' ':
        - Hello
        - World
```

</td>
<td> -> </td>
<td>

```xml
<Tag Attr="Foo,Bar;Hello World" />
```

</td>
</tr>
</table>

> Ok David now please provide an explanation and an apology for coming up with such a feature

Obviously this is not meant to be written by hand (maybe for like one level of nicer formatting of a longer list) but rather provide composition freedom with anchors and aliases. For example the delimiter object pattern comes handy in situations when a single list of items need to be fed to different systems, like the following made-up scenario:

```yml
# with flow scalars
- Pipeline:
  /artifacts: &artifacts
    - Distribution
    - DebugSymbols
    - TestResults
  :
  - Job: { Name: Build, Produces: { ', ': *artifacts } }:
    - Bash: { Command: { ' ': [ 'super-builder-9000 build', *artifacts ] } }
  - Job: { Name: Test, Consumes: { ', ': *artifacts } }:
    - Bash: { Command: { ' ': [ 'super-builder-9000 test', *artifacts ] } }
```

<details><summary>With block scalars</summary>

```yml
# with block scalars
- Pipeline:
  /artifacts: &artifacts
    - Distribution
    - DebugSymbols
    - TestResults
  :
  - Job:
      Name: Build
      Produces:
        ', ': *artifacts
    :
    - Bash:
        Command:
          ' ': [ 'super-builder-9000 build', *artifacts ]
  - Job:
      Name: Test
      Consumes:
        ', ': *artifacts
    :
    - Bash:
        Command:
          ' ': [ 'super-builder-9000 test', *artifacts ]
```

</details>

<details><summary>renders</summary>

```xml
<Pipeline>
  <Job Name="Build" Produces="Distribution, DebugSymbols, TestResults">
    <Bash Command="super-builder-9000 build Distribution DebugSymbols TestResults" />
  </Job>
  <Job Name="Test" Produces="Distribution, DebugSymbols, TestResults">
    <Bash Consumes="super-builder-9000 test Distribution DebugSymbols TestResults" />
  </Job>
</Pipeline>
```

</details>

## Special XML nodes

### Comments

In YAML comments are impossible to be part of the document tree, which is by design so it is impossible to give them semantics, however in XML they can be fit into the DOM. For the former reason many YAML parsers, even event based ones may ignore comments entirely as if they weren't there.

So preserving XML comments are done via special key `//` in YXX.

<table>
<tr>
<td>

```yaml
# This comment will be ignored entirely
- //: This comment will be preserved
```

</td>
<td> -> </td>
<td>

```xml
<!-- This comment will be preserved -->
```

</td>
</tr>
</table>

### Processing Instruction node

Much like custom YAML tags, processing instruction nodes are parser/application specific and have no standard formatting of their value. For this reason in YXX they're expressed as simple key-value pairs with `<?` prefix

<table>
<tr>
<td>

```yaml
- <?display: table-view
- <?sort: alpha-ascending
- <?textinfo: whitespace is allowed
- <?elementnames: <fred>, <bert>, <harry>
```

</td>
<td> -> </td>
<td>

```xml
<?display table-view?>
<?sort alpha-ascending?>
<?textinfo whitespace is allowed ?>
<?elementnames <fred>, <bert>, <harry> ?>
```

</td>
</tr>
</table>

### Declaration node

XML declaration node is syntactically a special case of a processing instruction node but many parsers treat it differently with common attributes. YXX also treats declaration node differently, by allowing attributes as a mapping.

<table>
<tr>
<td>

```yaml
# Simply put XML declaration before the root node.
- <?xml: { version: 1.0, encoding: utf-8 }
- MyRootNode: # ...
```

</td>
<td> -> </td>
<td>

```xml
<?xml version="1.0" encoding="utf-8"?>
<MyRootNode />
```

</td>
</tr>
</table>

### Doctype node

In YXX doctype nodes are handled the exact same way as processing instruction nodes but with `<!` prefix:

<table>
<tr>
<td>

```yaml
- <!DOCTYPE: html
```

</td>
<td> -> </td>
<td>

```xml
<!DOCTYPE html>
```

</td>
</tr>
</table>

# Bitter Recompense

> Comment: This looks cursed though, the examples are not more readable than their XML counterparts

Good point, but YXX was not really meant to be used in these toy examples fitting inside a readme file. Check out [this monstrosity](https://src.redpoint.games/redpointgames/unreal-engine-scripts/-/blob/main/Lib/BuildGraph_Project.xml?ref_type=heads) which had to be developed by hand (at least I didn't find any tool in that repository which would generate this). YXX ambition is making authoring such documents slightly less of a pain.

> Comment: What about Schemas?

Indeed generating a YAML schema from XSD for YXX is not a trivial task. Especially when the only available cross-platform/cross-parser YAML schema is simply derived from JSON schema which might make more advanced YAML features YXX relies on impossible to describe. To my knowledge currently there's no standard YAML schema which can describe complex keys, but please correct me if I'm wrong.
